__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__kernel void noneLocalMeans(
		__read_only image2d_t input,
		__write_only image2d_t output,
		//__private int group,
		//__private int window,
		__private float strength2,
		__private float sigma2
		
		){
    
    const int dataType = get_image_channel_data_type(input);
    //const float pi = 3.14159265359;
    const int2 iD = get_image_dim(input);
    const int2 pos = {get_global_id(0), get_global_id(1)};
    
    //variables
    //const int g2 = ((GROUP+1)*2)*((GROUP+1)*2)-1;
    float normSum = 0.0f;
    float totSum = 0.0f;
    float value = 0.0f;
    float indi = 0.0f;
    float groupTot = 0.0f;
    
    
    for(int i = pos.x - WINDOW; i < pos.x + WINDOW; i++){
        for(int j = pos.y - WINDOW; j < pos.y + WINDOW; j++){
            int mX = pos.x - GROUP;
            int mY = pos.y - GROUP;
            
            if(i != pos.x && j != pos.y){
                for(int k = i - GROUP; k < i + GROUP; k++,mX++){
                    for(int l = j - GROUP; l < j + GROUP; l++,mY++){
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
                        indi = native_exp( - (native_divide(indi,strength2)));
                        groupTot += indi;
                    }
                    
                }
                int2 coord = {i,j};
                if(dataType == CLK_FLOAT){
                    value = read_imagef(input,sampler,coord).x;
                }else if(dataType == CLK_UNSIGNED_INT8 || dataType == CLK_UNSIGNED_INT16){
                    value = read_imageui(input,sampler,coord).x;
                }else{
                    value = read_imagei(input,sampler,coord).x;
                }

                float2 dist = convert_float(coord - pos);
                float gaussWeight = native_exp( - native_divide( dot(dist, dist), (2.0f * sigma2) ) );
                //gaussWeight = native_divide(gaussWeight, 2.0f * pi * sigma2);
                gaussWeight = native_divide(gaussWeight, 2.0f * sigma2);
                groupTot *= gaussWeight;
                
                normSum += groupTot;
                totSum += groupTot * value;
                groupTot = 0.0f;
            }
        }
    }
    value = native_divide(totSum,normSum);
    if(value < 0){
        value = 0;
    }
    if(value > 1.0){
        value = 1.0f;
    }
    
    int outputDataType = get_image_channel_data_type(output);
    if(outputDataType == CLK_FLOAT) {
        write_imagef(output, pos, value);
    } else if(outputDataType == CLK_UNSIGNED_INT8 || outputDataType == CLK_UNSIGNED_INT16) {
        write_imageui(output, pos, round(value));
    } else {
        write_imagei(output, pos, round(value));
    }
   
}
