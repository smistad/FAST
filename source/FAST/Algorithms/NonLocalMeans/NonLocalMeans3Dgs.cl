//#pragma OPENCL EXTENSION cl_khr_3d_image_writes : enable
__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

#ifdef cl_khr_3d_image_writes
__kernel void noneLocalMeans(
		__read_only image3d_t input,
		__write_only image3d_t output,
		__private float strength2,
		__private float sigma2
		){
    
	const int dataType = get_image_channel_data_type(input);
    
    //const int4 iD = get_image_dim(input);
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2),0};
    
    //Variables
    const int g3 = ((GROUP+1)*2)*((GROUP+1)*2)*((GROUP+1)*2);
    float totSum = 0.0f;
    float normSum = 0.0f;
    float value = 0.0f;
    float groupTot = 0.0f;
    float indi = 0.0f;
    const float K = native_divide(1.0f,g3);
    
    for(int i = pos.x-WINDOW; i <= pos.x + WINDOW; i++){
        for(int j = pos.y - WINDOW; j <= pos.y + WINDOW; j++){
            for(int k = pos.z - WINDOW; k <= pos.z + WINDOW; k++){
                int mX = pos.x - GROUP;
                int mY = pos.y - GROUP;
                int mZ = pos.z - GROUP;
                if(i != pos.x && j != pos.y && k != pos.z){
                    for(int l = i - GROUP; l <= i + GROUP; l++, mX++){
                        for(int m = j - GROUP; m <= j + GROUP; m++, mY++){
                            for(int n = k - GROUP; n <= k + GROUP; n++, mZ++){
                                int4 mPos = {mX,mY,mZ,0};
                                int4 sPos = {l,m,n,0};
                                
                                if(dataType == CLK_FLOAT){
                                    indi = read_imagef(input, sampler,mPos).x - read_imagef(input, sampler, sPos).x;
                                }else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16){
                                    indi = read_imageui(input, sampler,mPos).x - read_imageui(input, sampler, sPos).x;
                                }else{
                                    indi = read_imagei(input, sampler,mPos).x - read_imagei(input, sampler, sPos).x;
                                }
                                
                                //indi = fabs(indi*indi);
                                indi = indi * indi;
                                
                                groupTot += indi;
                                //mZ++;
                            }
                            //mY++;
                        }
                        //mX++;
                    }
                    int4 coord = {i,j,k,0};
                    
                    if(dataType == CLK_FLOAT){
                        value = read_imagef(input,sampler,coord).x;
                    }else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16){
                        value = read_imageui(input,sampler,coord).x;
                    }else{
                        value = read_imagei(input,sampler,coord).x;
                    }
                    groupTot = K * groupTot;
                    groupTot = native_exp(-groupTot/strength2);
                    normSum += groupTot;
                    totSum += groupTot*value;
                    groupTot = 0.0f;
                }
            }
        }
    }
    value = native_divide(totSum,normSum);
    
    
    int outputDataType = get_image_channel_data_type(output);
    if(outputDataType == CLK_FLOAT) {
        float holder = native_divide(totSum,normSum);
        if(holder > 0){
            value = holder;
        }else if(holder > 1){
            value = 1.0f;
        }else{
            value = 0.0f;
        }
        write_imagef(output,pos,value);
    } else if(outputDataType == CLK_UNSIGNED_INT8 || outputDataType == CLK_UNSIGNED_INT16) {
        int holder = 0;
        if(value > -1 && value < 2){
            holder = value * 255;
        }else if(value > 1){
            holder = 255;
        }
        holder = min(max(holder,0),255);
        write_imageui(output, pos, holder);
    } else {
        int holder = 0;
        if(value > -1 && value < 2){
            holder = value * 255;
        }else if(value > 1){
            holder = 255;
        }
        holder = min(max(holder,0),255);
        write_imagei(output, pos, holder);
    }
}
#else
__kernel void noneLocalMeans(
        __read_only image3d_t input,
        __global TYPE* output,
        __private float strength2,
        __private float sigma2
        ){
    
    const int dataType = get_image_channel_data_type(input);
    
    //const int4 iD = get_image_dim(input);
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2),0};
    
    const int g3 = ((GROUP+1)*2)*((GROUP+1)*2)*((GROUP+1)*2);
    float totSum = 0.0f;
    float normSum = 0.0f;
    float value = 0.0f;
    float groupTot = 0.0f;
    float indi = 0.0f;
    const float K = native_divide(1.0f,g3);
    
    for(int i=pos.x-WINDOW; i <= pos.x + WINDOW; i++){
        for(int j = pos.y - WINDOW; j <= pos.y + WINDOW; j++){
            for(int k = pos.z - WINDOW; k <= pos.z + WINDOW; k++){
                int mX = pos.x - GROUP;
                int mY = pos.y - GROUP;
                int mZ = pos.z - GROUP;
                if(i != pos.x && j != pos.y && k != pos.z){
                    for(int l = i - GROUP; l <= i + GROUP; l++, mX++){
                        for(int m = j - GROUP; m <= j + GROUP; m++, mY++){
                            for(int n = k - GROUP; n <= k + GROUP; n++, mZ++){
                                int4 mPos = {mX,mY,mZ,0};
                                int4 sPos = {l,m,n,0};
                                
                                if(dataType == CLK_FLOAT){
                                    indi = read_imagef(input, sampler,mPos).x - read_imagef(input, sampler, sPos).x;
                                }else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16){
                                    indi = read_imageui(input, sampler,mPos).x - read_imageui(input, sampler, sPos).x;
                                }else{
                                    indi = read_imagei(input, sampler,mPos).x - read_imagei(input, sampler, sPos).x;
                                }
                                indi = indi * indi;
                                //indi = fabs(indi*indi);
                                
                                groupTot += indi;
                                //mZ++;
                            }
                            //mY++;
                        }
                        //mX++;
                    }
                    int4 coord = {i,j,k,0};
                    
                    if(dataType == CLK_FLOAT){
                        value = read_imagef(input,sampler,coord).x;
                    }else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16){
                        value = read_imageui(input,sampler,coord).x;
                    }else{
                        value = read_imagei(input,sampler,coord).x;
                    }
                    groupTot = K * groupTot;
                    groupTot = native_exp(-groupTot/strength2);
                    normSum += groupTot;
                    totSum += groupTot*value;
                    groupTot = 0.0f;
                }
            }
        }
    }
    value = native_divide(totSum,normSum);
    output[pos.x+pos.y*get_global_size(0)+pos.z*get_global_size(0)*get_global_size(1)] = value;
}
#endif
