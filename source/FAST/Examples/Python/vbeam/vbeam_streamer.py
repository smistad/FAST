from time import sleep
from typing import Union
import fast
import jax.numpy as np
import pyuff_ustb as pyuff
import vbeam.scan
from vbeam.data_importers import import_pyuff, parse_pyuff_scan
from vbeam.scan import sector_scan, linear_scan, LinearScan, SectorScan
import jax
from vbeam.beamformers import get_das_beamformer


class VBeamStreamer(fast.PythonRandomAccessStreamer):
    """
    Stream data from VBeam beamformer
    """
    def __init__(self, uff_file: str, grid_width: int = 256, grid_depth: int = 512, framerate: int = 2,
                 scan: Union[vbeam.scan.Scan, None] = None, max_depth: Union[float, None] = None, is_sector_scan: Union[bool, None] = None):
        super().__init__()
        self.createOutputPort(0)
        self.uff_file_path = uff_file
        self._data = None
        self.channel_data = None
        self.scan = scan
        self.grid_width = grid_width
        self.grid_depth = grid_depth
        self.N_frames = None
        self.setFramerate(framerate)
        self.is_sector_scan = is_sector_scan
        self.max_depth = max_depth

    def getNrOfFrames(self):
        """
        This function must return how many frames the streamer has.
        :return: nr of frames
        """
        if self.N_frames is None:
            self.initialize()
        return self.N_frames

    def initialize(self):
        """
        Read from UFF file
        :return:
        """
        print('Reading UFF ...')
        uff = pyuff.Uff(self.uff_file_path)
        self.channel_data = uff.read("/channel_data")
        self.N_frames = self.channel_data.N_frames
        try:
            self.scan = parse_pyuff_scan(uff.read('/scan'))
            print('Got scan from UFF file.')
        except:
            # Scan is missing from file
            print('Scan was not found in the UFF file.')

    def generateStream(self):
        if self.channel_data is None:
            self.initialize()

        # Beamform
        setup = import_pyuff(self.channel_data, scan=self.scan)

        frame_metadata = {}
        if self.max_depth is None:
            max_depth = self.channel_data.N_samples * (
                    1.0 / self.channel_data.sampling_frequency) * self.channel_data.sound_speed * 0.5
        else:
            max_depth = self.max_depth
        if self.scan is None:
            if self.is_sector_scan is None:
                raise ValueError('scan or is_sector_scan was not provided to VBeamStreamer constructor. Please provide one of these.')
            elif self.is_sector_scan:
                print(f'No scan provided. Creating a sector scan with max_depth {max_depth}, and grid size {self.grid_width}x{self.grid_depth}')
                scan_angles = np.array([wave.source.azimuth for wave in self.channel_data.sequence])
                scan_depths = np.linspace(0, max_depth, self.grid_depth)
                scan = sector_scan(scan_angles, scan_depths).resize(azimuths=self.grid_width)
            else:
                print(f'No scan provided. Creating a linear scan with max_depth {max_depth}, and grid size {self.grid_width}x{self.grid_depth}')
                scan_lines = np.linspace(self.channel_data.probe.geometry[0, 0], self.channel_data.probe.geometry[0, -1], self.grid_width)
                scan_depths = np.linspace(self.channel_data.probe.element_height, max_depth, self.grid_depth)
                scan = linear_scan(scan_lines, scan_depths)
            setup.scan = scan
        else:
            setup.scan = self.scan
        if isinstance(setup.scan, LinearScan):
            frame_metadata['isPolar'] = 'false'
            frame_metadata['startRadius'] = str(setup.scan.z[0])
            frame_metadata['stopRadius'] = str(setup.scan.z[-1])
            frame_metadata['startTheta'] = str(setup.scan.x[0])
            frame_metadata['stopTheta'] = str(setup.scan.x[-1])
        elif isinstance(setup.scan, SectorScan):
            scan_angles = np.array([wave.source.azimuth for wave in self.channel_data.sequence])
            frame_metadata['isPolar'] = 'true'
            frame_metadata['startRadius'] = str(0)
            frame_metadata['stopRadius'] = str(max_depth)
            frame_metadata['startTheta'] = str(scan_angles[0])
            frame_metadata['stopTheta'] = str(scan_angles[-1])
        print('Setting up beamformer ...')
        beamformer = jax.jit(get_das_beamformer(setup, scan_convert=False, log_compress=False))
        print('Beamforming now ...')
        self._data = beamformer(**setup.data)
        print(self._data.shape)
        if len(self._data.shape) == 2: # Missing temporal dimension because only 1 frame, add it:
            self._data = np.expand_dims(self._data, axis=0)
        print('Beamforming done')

        while not self.isStopped():
            # First, we need to check if this streaming is paused
            if self.getPause():
                self.waitForUnpause() # Wait for streamer to be unpaused
            pause = self.getPause() # Check whether to pause or not
            frame = self.getCurrentFrameIndex()

            data = self._data[frame, ...].T
            data2 = np.transpose(np.stack([data.real, data.imag]), axes=(1,2,0)) # Rearrange data for FAST
            image = fast.Image.createFromArray(data2._value) # ndarray data is in _value
            image.setFrameData(frame_metadata)
            if frame == self.getNrOfFrames()-1: # If this is last frame, mark it as such
                image.setLastFrame('VBeamStreamer')

            if not pause:
                if self.getFramerate() > 0:
                    sleep(1.0/self.getFramerate()) # Sleep to give the requested framerate
                self.getCurrentFrameIndexAndUpdate() # Update the frame index to the next frame
            try:
                self.addOutputData(0, image)
                self.frameAdded() # Important to notify any listeners
            except:
                break

    def refresh(self):
        # FIXME Get a 1 frame glitch when first doing this. Old frame in memory?
        if self.getPause():
            self.setCurrentFrameIndex(self.getCurrentFrameIndex())
