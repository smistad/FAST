__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

__kernel void noneLocalMeans(
		__read_only image2d_t input,
		__write_only image2d_t output,
		//__private int group,
		//__private int window,
		__private float strength,
		__private float sigma,
		__private int k
		){
    
	const int dataType = get_image_channel_data_type(input);
    
    const int2 iD = get_image_dim(input);
    const int2 pos = {get_global_id(0), get_global_id(1)};
    
	//variables
    //const int hG = (group-1)/2;
	const int g2 = ((GROUP+1)*2)*((GROUP+1)*2);
    //const int hW = (window-1)/2;
    float normSum = 0.0f;
    float totSum = 0.0f;
	float value = 0.0f;
	float indi = 0.0f;
	float groupTot = 0.0f;
    const float strength2 = strength * strength;
	const float K = native_divide(1.0f,g2);

    int mX = 0;
    int mY = 0;
    
	if(((pos.x + WINDOW + GROUP) > iD.x + 1) || ((pos.x - WINDOW - GROUP) < 0) || ((pos.y + WINDOW + GROUP) > iD.y+1) || ((pos.y - WINDOW - GROUP) < 0)){
        for(int i=pos.x-WINDOW; i < pos.x + WINDOW; i++){
            for(int j = pos.y - WINDOW; j < pos.y + WINDOW; j++){
                int mX = pos.x - GROUP;
                int mY = pos.y - GROUP;
                if(i != pos.x && j != pos.y){
                    for(int k = i - GROUP; k < i + GROUP; k++){
                        for(int l = j - GROUP; l < j + GROUP; l++){
                            int2 mPos = {mX,mY};
                            int2 sPos = {k,l};
                            if(k < 0){sPos.x = abs(k);}
                            if(l < 0){sPos.y = abs(l);}
                            if(mX < 0){mPos.x = abs(mX);}
                            if(mY < 0){mPos.y = abs(mY);}
                            if(k > iD.x-1){sPos.x = iD.x - (sPos.x - iD.x);}
                            if(l > iD.y-1){sPos.y = iD.y - (sPos.y - iD.y);}
                            if(mX > iD.x-1){mPos.x = iD.x - (mPos.x - iD.x);}
                            if(mY > iD.y-1){mPos.y = iD.y - (mPos.y - iD.y);}
                            
                            if(dataType == CLK_FLOAT){
                                indi = read_imagef(input, sampler,mPos).x - read_imagef(input, sampler, sPos).x;
                            }else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16){
                                indi = read_imageui(input, sampler,mPos).x - read_imageui(input, sampler, sPos).x;
                            }else{
                                indi = read_imagei(input, sampler,mPos).x - read_imagei(input, sampler, sPos).x;
                            }
                            
                            indi = fabs(indi);
                            indi = indi * indi;
                            
                            if(KVERSION == 1 || KVERSION == 2){
                                //K = gamle versjonen
                                indi = native_exp( - (native_divide(indi,strength2)));
                                groupTot += indi;
                            }if(KVERSION == 3){
                                //K = original gauss
                                float mid = (indi - 2*SIGMA2);
                                groupTot += mid;
                            }if(KVERSION == 4){
                                // K = 0 + K = 3
                                groupTot += K * (indi - 2*SIGMA2);
                            }if(KVERSION == 5){
                                //Neighborhood distance
                                //int mDist1 = abs(k-i) * abs(l-j);
                                //mDist1 += mDist1;
                                //mDist1 = native_sqrt((float)mDist1);
                                int mDist1 = abs(k-i) + abs(l-j);
                                float mIndi = 0.0f;
                                for(int m=-1; m < mDist1; m++){
                                    mIndi += (1/((2*m+1)*(2*m+1)));
                                }
                                mIndi = (1/GROUP)*mIndi;
                                groupTot += indi*mIndi;
                            }else{
                                //K = 1/d2
                                groupTot += K * indi;
                            }
                            mY++;
                        }
                        mX++;
                    }
                    
                    int2 coord = {i,j};
                    if(i < 0){coord.x = abs(i);}
                    if(j < 0){coord.y = abs(j);}
                    if(i > iD.x-1){coord.x = iD.x - (coord.x - iD.x);}
                    if(j > iD.y-1){coord.y = iD.y - (coord.y - iD.y);}
                    
                    if(dataType == CLK_FLOAT){
                        value = read_imagef(input,sampler,coord).x;
                    }else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16){
                        value = read_imageui(input,sampler,coord).x;
                    }else{
                        value = read_imagei(input,sampler,coord).x;
                    }
                    if(KVERSION == 2){ //Performs sqrt for euclid dist
                        int eDist = ((pos.x - coord.x) * (pos.x - coord.x)) + ((pos.y - coord.y) * (pos.y - coord.y));
                        float sDist = native_sqrt((float)abs(eDist));
                        float gWeight = native_exp( - native_divide( sDist, (2.0f * (float)SIGMA2) ) );
                        groupTot *= (1.0f / (2.0f * M_PI * SIGMA2) * gWeight);
                    }else if(KVERSION == 1){ //Skips sqrt for euclid dist
                        int eDist = ((pos.x - coord.x) * (pos.x - coord.x)) + ((pos.y - coord.y) * (pos.y - coord.y));
                        float sDist = native_sqrt((float)abs(eDist));
                        float gWeight = native_exp( - native_divide( sDist, (2.0f * (float)SIGMA2) ) );
                        groupTot *= (1.0f / (2.0f * M_PI * SIGMA2) * gWeight);
                    }else{
                        groupTot = native_exp(-groupTot/strength2);
                        
                    }
                    normSum += groupTot;
                    totSum += groupTot*value;
                    groupTot = 0.0f;
                }
            }
        }
	}else{
		for(int i = pos.x - WINDOW; i < pos.x + WINDOW; i++){
			for(int j = pos.y - WINDOW; j < pos.y + WINDOW; j++){
				mX = i - GROUP;
				mY = j - GROUP;
				
				if(i != pos.x && j != pos.y){
					for(int k = i - GROUP; k < i + GROUP; k++){
						for(int l = j - GROUP; l < j + GROUP; l++){
							int2 mPos = {mX,mY};
							int2 sPos = {k,l};

							if(dataType == CLK_FLOAT){
								indi = read_imagef(input, sampler,mPos).x - read_imagef(input, sampler, sPos).x;
							}else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16){
								indi = read_imageui(input, sampler,mPos).x - read_imageui(input, sampler, sPos).x;
							}else{
								indi = read_imagei(input, sampler,mPos).x - read_imagei(input, sampler, sPos).x;
							}
							indi = fabs(indi*indi);
							groupTot += K * indi;
							mY++;
						}
						mX++;
					}
					int2 coord = {i,j};
					if(dataType == CLK_FLOAT){
						value = read_imagef(input,sampler,coord).x;
					}else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16){
						value = read_imageui(input,sampler,coord).x;
					}else{
						value = read_imagei(input,sampler,coord).x;
					}
					groupTot = native_exp(-groupTot/strength2);
					normSum += groupTot;
					totSum += groupTot * value;
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
