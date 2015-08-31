__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
__constant sampler_t interpolationSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;
__constant sampler_t hpSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

#define LPOS(pos) pos.x+pos.y*get_global_size(0)+pos.z*get_global_size(0)*get_global_size(1)

#ifdef VECTORS_16BIT
#define UNORM16_TO_FLOAT(v) (float)v / 65535.0f
#define FLOAT_TO_UNORM16(v) convert_ushort_sat_rte(v * 65535.0f)
#define TDF_TYPE ushort
#else
#define UNORM16_TO_FLOAT(v) v
#define FLOAT_TO_UNORM16(v) v
#define TDF_TYPE float
#endif

float4 readImageToFloat(
        __read_only image3d_t volume, 
        int4 position
    ) {
    int dataType = get_image_channel_data_type(volume);
    float4 value;
    if(dataType == CLK_FLOAT) {
        value = read_imagef(volume, sampler, position).x; 
    } else if(dataType == CLK_SIGNED_INT16 || dataType == CLK_SIGNED_INT8) {
        value = convert_float4(read_imagei(volume, sampler, position)); 
    } else {
        value = convert_float4(read_imageui(volume, sampler, position)); 
    }
    return value;
}

float3 gradient(
        __read_only image3d_t volume,   // Volume to perform gradient on
        int4 pos,                       // Position to perform gradient on
        int volumeComponent,            // The volume component to perform gradient on: 0, 1 or 2
        int dimensions                  // The number of dimensions to perform gradient in: 1, 2 or 3
    ) {
    float f100, f_100, f010, f0_10, f001, f00_1;
    switch(volumeComponent) {
        case 0:
        f100 = read_imagef(volume, pos + (int4)(1,0,0,0)).x; 
        f_100 = read_imagef(volume, pos - (int4)(1,0,0,0)).x;
        if(dimensions > 1) {
            f010 = read_imagef(volume, pos + (int4)(0,1,0,0)).x; 
            f0_10 = read_imagef(volume, pos - (int4)(0,1,0,0)).x;
        }
        if(dimensions > 2) {
            f001 = read_imagef(volume, pos + (int4)(0,0,1,0)).x;
            f00_1 = read_imagef(volume, pos - (int4)(0,0,1,0)).x;
        }
    break;
        case 1:
        f100 = read_imagef(volume, pos + (int4)(1,0,0,0)).y;
        f_100 = read_imagef(volume, pos - (int4)(1,0,0,0)).y;
        if(dimensions > 1) {
            f010 = read_imagef(volume, pos + (int4)(0,1,0,0)).y;
            f0_10 = read_imagef(volume, pos - (int4)(0,1,0,0)).y;
        }
        if(dimensions > 2) {
            f001 = read_imagef(volume, pos + (int4)(0,0,1,0)).y;
            f00_1 = read_imagef(volume, pos - (int4)(0,0,1,0)).y;
        }
    break;
        case 2:
        f100 = read_imagef(volume, pos + (int4)(1,0,0,0)).z;
        f_100 = read_imagef(volume, pos - (int4)(1,0,0,0)).z;
        if(dimensions > 1) {
            f010 = read_imagef(volume, pos + (int4)(0,1,0,0)).z;
            f0_10 = read_imagef(volume, pos - (int4)(0,1,0,0)).z;
        }
        if(dimensions > 2) {
            f001 = read_imagef(volume, pos + (int4)(0,0,1,0)).z;
            f00_1 = read_imagef(volume, pos - (int4)(0,0,1,0)).z;
        }
    break;
    }

    // TODO spacing here..
    float3 grad = {
        0.5f*(f100-f_100), 
        0.5f*(f010-f0_10),
        0.5f*(f001-f00_1)
    };

    return grad;
}

float3 gradientNormalized(
        __read_only image3d_t volume,   // Volume to perform gradient on
        int4 pos,                       // Position to perform gradient on
        int volumeComponent,            // The volume component to perform gradient on: 0, 1 or 2
        int dimensions                  // The number of dimensions to perform gradient in: 1, 2 or 3
    ) {
    float f100, f_100, f010, f0_10, f001, f00_1;
    switch(volumeComponent) {
        case 0:
        f100 = read_imagef(volume, pos + (int4)(1,0,0,0)).x; 
        f_100 = read_imagef(volume, pos - (int4)(1,0,0,0)).x;
        if(dimensions > 1) {
            f010 = read_imagef(volume, pos + (int4)(0,1,0,0)).x; 
            f0_10 = read_imagef(volume, pos - (int4)(0,1,0,0)).x;
        }
        if(dimensions > 2) {
            f001 = read_imagef(volume, pos + (int4)(0,0,1,0)).x;
            f00_1 = read_imagef(volume, pos - (int4)(0,0,1,0)).x;
        }
    break;
        case 1:
        f100 = read_imagef(volume, pos + (int4)(1,0,0,0)).y;
        f_100 = read_imagef(volume, pos - (int4)(1,0,0,0)).y;
        if(dimensions > 1) {
            f010 = read_imagef(volume, pos + (int4)(0,1,0,0)).y;
            f0_10 = read_imagef(volume, pos - (int4)(0,1,0,0)).y;
        }
        if(dimensions > 2) {
            f001 = read_imagef(volume, pos + (int4)(0,0,1,0)).y;
            f00_1 = read_imagef(volume, pos - (int4)(0,0,1,0)).y;
        }
    break;
        case 2:
        f100 = read_imagef(volume, pos + (int4)(1,0,0,0)).z;
        f_100 = read_imagef(volume, pos - (int4)(1,0,0,0)).z;
        if(dimensions > 1) {
            f010 = read_imagef(volume, pos + (int4)(0,1,0,0)).z;
            f0_10 = read_imagef(volume, pos - (int4)(0,1,0,0)).z;
        }
        if(dimensions > 2) {
            f001 = read_imagef(volume, pos + (int4)(0,0,1,0)).z;
            f00_1 = read_imagef(volume, pos - (int4)(0,0,1,0)).z;
        }
    break;
    }
    
    float3 grad = {
        0.5f*(f100/length(read_imagef(volume, sampler, pos + (int4)(1,0,0,0)).xyz)
                - f_100/length(read_imagef(volume, sampler, pos - (int4)(1,0,0,0)).xyz)), 
        0.5f*(f010/length(read_imagef(volume, sampler, pos + (int4)(0,1,0,0)).xyz) -
                f0_10/length(read_imagef(volume, sampler, pos - (int4)(0,1,0,0)).xyz)),
        0.5f*(f001/length(read_imagef(volume, sampler, pos + (int4)(0,0,1,0)).xyz) -
                f00_1/length(read_imagef(volume, sampler, pos - (int4)(0,0,1,0)).xyz))
    };


    return grad;
}

__kernel void toFloat(
        __read_only image3d_t volume,
#ifdef cl_khr_3d_image_writes
        __write_only image3d_t processedVolume,
#else
        __global float* processedVolume,
#endif
        __private float minimum,
        __private float maximum
        ) {
    int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    
    float v = readImageToFloat(volume, pos).x;

    v = v > maximum ? maximum : v;
    v = v < minimum ? minimum : v;

    // Convert to floating point representation 0 to 1
    float value = (v - minimum) / (maximum - minimum);

    // Store value
#ifdef cl_khr_3d_image_writes
    write_imagef(processedVolume, pos, value);
#else
    processedVolume[LPOS(pos)] = value;
#endif
}

__kernel void createVectorField(
        __read_only image3d_t volume, 
#ifdef cl_khr_3d_image_writes
        __write_only image3d_t vectorField, 
#else
        __global float* vectorField,
#endif
        __private float Fmax,
        __private float vsign
        ) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};

    // Gradient of volume
    float4 F; 
    F.xyz = vsign*gradient(volume, pos, 0, 3); // The sign here is important
    F.w = 0.0f;

    // Fmax normalization
    const float l = length(F);
    F = l < Fmax ? F/(Fmax) : F / (l);
    F.w = 1.0f;

    // Store vector field
#ifdef cl_khr_3d_image_writes
    write_imagef(vectorField, pos, F);
#else
    vstore3(F.xyz, LPOS(pos), vectorField);
#endif
}

// Forward declaration of eigen_decomp function
void eigen_decomposition(float M[3][3], float V[3][3], float e[3]);

__constant float cosValues[32] = {1.0f, 0.540302f, -0.416147f, -0.989992f, -0.653644f, 0.283662f, 0.96017f, 0.753902f, -0.1455f, -0.91113f, -0.839072f, 0.0044257f, 0.843854f, 0.907447f, 0.136737f, -0.759688f, -0.957659f, -0.275163f, 0.660317f, 0.988705f, 0.408082f, -0.547729f, -0.999961f, -0.532833f, 0.424179f, 0.991203f, 0.646919f, -0.292139f, -0.962606f, -0.748058f, 0.154251f, 0.914742f};
__constant float sinValues[32] = {0.0f, 0.841471f, 0.909297f, 0.14112f, -0.756802f, -0.958924f, -0.279415f, 0.656987f, 0.989358f, 0.412118f, -0.544021f, -0.99999f, -0.536573f, 0.420167f, 0.990607f, 0.650288f, -0.287903f, -0.961397f, -0.750987f, 0.149877f, 0.912945f, 0.836656f, -0.00885131f, -0.84622f, -0.905578f, -0.132352f, 0.762558f, 0.956376f, 0.270906f, -0.663634f, -0.988032f, -0.404038f};

__kernel void circleFittingTDF(
        __read_only image3d_t vectorField,
        __global TDF_TYPE * T,
        //__global float * Radius,
        __private float rMin,
        __private float rMax,
        __private float rStep
    ) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};

    // Find Hessian Matrix
    float3 Fx, Fy, Fz;
    if(rMax < 4) {
        Fx = gradient(vectorField, pos, 0, 1);
        Fy = gradient(vectorField, pos, 1, 2);
        Fz = gradient(vectorField, pos, 2, 3);
    } else {
        Fx = gradientNormalized(vectorField, pos, 0, 1);
        Fy = gradientNormalized(vectorField, pos, 1, 2);
        Fz = gradientNormalized(vectorField, pos, 2, 3);
    }


    float Hessian[3][3] = {
        {Fx.x, Fy.x, Fz.x},
        {Fy.x, Fy.y, Fz.y},
        {Fz.x, Fz.y, Fz.z}
    };
    
    // Eigen decomposition
    float eigenValues[3];
    float eigenVectors[3][3];
    eigen_decomposition(Hessian, eigenVectors, eigenValues);
    //const float3 lambda = {eigenValues[0], eigenValues[1], eigenValues[2]};
    //const float3 e1 = {eigenVectors[0][0], eigenVectors[1][0], eigenVectors[2][0]};
    const float3 e2 = {eigenVectors[0][1], eigenVectors[1][1], eigenVectors[2][1]};
    const float3 e3 = {eigenVectors[0][2], eigenVectors[1][2], eigenVectors[2][2]};

    /*
    if(lambda.y > 0 && lambda.z > 0) {
        T[LPOS(pos)] = 0;
        return;
    }
    */

    // Circle Fitting
    float maxSum = 0.0f;
    float maxRadius = 0.0f;
    const float4 floatPos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    for(float radius = rMin; radius <= rMax; radius += rStep) {
        float radiusSum = 0.0f;
        int samples = 32;
        int stride = 1;
        //int negatives = 0;
        /*
        if(radius < 3) {
            samples = 8;
            stride = 4;
        } else if(radius < 6) {
            samples = 16;
            stride = 2;
        }
        */

        for(int j = 0; j < samples; j++) {
            float3 V_alpha = cosValues[j*stride]*e3 + sinValues[j*stride]*e2;
            float4 position = floatPos + radius*V_alpha.xyzz;
            float3 V = -read_imagef(vectorField, interpolationSampler, position).xyz;
            radiusSum += dot(V, V_alpha);
        }
        radiusSum /= samples;
        if(radiusSum > maxSum) {
            maxSum = radiusSum;
            maxRadius = radius;
        } else {
            break;
        }
    }

    // Store result
    T[LPOS(pos)] = FLOAT_TO_UNORM16(maxSum);
    //Radius[LPOS(pos)] = maxRadius;
}

__kernel void nonCircularTDF(
        __read_only image3d_t vectorField,
        __global TDF_TYPE * T,
        __private float rMin,
        __private float rMax,
        __private float rStep,
        __private const int arms,
        //__global float * R,
        __private const float minAverageMag
    ) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    int invalid = 0;

    // Find Hessian Matrix
    const float3 Fx = gradientNormalized(vectorField, pos, 0, 1);
    const float3 Fy = gradientNormalized(vectorField, pos, 1, 2);
    const float3 Fz = gradientNormalized(vectorField, pos, 2, 3);

    float Hessian[3][3] = {
        {Fx.x, Fy.x, Fz.x},
        {Fy.x, Fy.y, Fz.y},
        {Fz.x, Fz.y, Fz.z}
    };

    // Eigen decomposition
    float eigenValues[3];
    float eigenVectors[3][3];
    eigen_decomposition(Hessian, eigenVectors, eigenValues);
    const float3 e1 = {eigenVectors[0][0], eigenVectors[1][0], eigenVectors[2][0]};
    const float3 e2 = {eigenVectors[0][1], eigenVectors[1][1], eigenVectors[2][1]};
    const float3 e3 = {eigenVectors[0][2], eigenVectors[1][2], eigenVectors[2][2]};

    float currentVoxelMagnitude = length(read_imagef(vectorField, sampler, pos).xyz);

    float maxRadius[12]; // 12 is maximum nr of arms atm.
    float sum = 0.0f;
    //float minAverageMag = 0.01f; // 0.01
    float avgRadius = 0.0f;
    for(int j = 0; j < arms; j++) {
        maxRadius[j] = 999;
        float alpha = 2 * M_PI_F * j / arms;
        float4 V_alpha = cos(alpha)*e3.xyzz + sin(alpha)*e2.xyzz;
        float prevMagnitude2 = currentVoxelMagnitude;
        float4 position = convert_float4(pos) + rMin*V_alpha;
        float prevMagnitude = length(read_imagef(vectorField, interpolationSampler, position).xyz);
        int up = prevMagnitude2 > prevMagnitude ? 0 : 1;

        // Perform the actual line search
        for(float radius = rMin+rStep; radius <= rMax; radius += rStep) {
            position = convert_float4(pos) + radius*V_alpha;
            float4 vec = read_imagef(vectorField, interpolationSampler, position);
            vec.w = 0.0f;
            float magnitude = length(vec.xyz);

            // Is a border point found?
            if(up == 1 && magnitude < prevMagnitude && (prevMagnitude+magnitude)/2.0f - currentVoxelMagnitude > minAverageMag) { // Dot produt here is test
                maxRadius[j] = radius;
                avgRadius += radius;
                if(dot(normalize(vec.xyz), -normalize(V_alpha.xyz)) < 0.0f) {
                    invalid = 1;
                    sum = 0.0f;
                    //break;
                }
                sum += 1.0f-fabs(dot(normalize(vec.xyz), e1));
                break;
            } // End found border point

            if(magnitude > prevMagnitude) {
                up = 1;
            }
            prevMagnitude = magnitude;
        } // End for each radius

        if(maxRadius[j] == 999 || invalid == 1) {
            invalid = 1;
            break;
        }
    } // End for arms

    avgRadius = avgRadius / arms;

    //R[LPOS(pos)] = avgRadius;
    if(invalid != 1) {
        float avgSymmetry = 0.0f;
        for(int j = 0; j < arms/2; j++) {
           avgSymmetry += min(maxRadius[j], maxRadius[arms/2 + j]) /
                max(maxRadius[j], maxRadius[arms/2+j]);
        }
        avgSymmetry /= arms/2;
        T[LPOS(pos)] = FLOAT_TO_UNORM16(min(1.0f, (sum / (arms))*avgSymmetry+0.2f));
    } else {
        T[LPOS(pos)] = 0;
    }
}

#define MAX(a, b) ((a)>(b)?(a):(b))

#define SIZE 3

float hypot2(float x, float y) {
  return sqrt(x*x+y*y);
}

// Symmetric Householder reduction to tridiagonal form.

void tred2(float V[SIZE][SIZE], float d[SIZE], float e[SIZE]) {

//  This is derived from the Algol procedures tred2 by
//  Bowdler, Martin, Reinsch, and Wilkinson, Handbook for
//  Auto. Comp., Vol.ii-Linear Algebra, and the corresponding
//  Fortran subroutine in EISPACK.

  for (int j = 0; j < SIZE; j++) {
    d[j] = V[SIZE-1][j];
  }

  // Householder reduction to tridiagonal form.

  for (int i = SIZE-1; i > 0; i--) {

    // Scale to avoid under/overflow.

    float scale = 0.0f;
    float h = 0.0f;
    for (int k = 0; k < i; k++) {
      scale = scale + fabs(d[k]);
    }
    if (scale == 0.0f) {
      e[i] = d[i-1];
      for (int j = 0; j < i; j++) {
        d[j] = V[i-1][j];
        V[i][j] = 0.0f;
        V[j][i] = 0.0f;
      }
    } else {

      // Generate Householder vector.

      for (int k = 0; k < i; k++) {
        d[k] /= scale;
        h += d[k] * d[k];
      }
      float f = d[i-1];
      float g = sqrt(h);
      if (f > 0) {
        g = -g;
      }
      e[i] = scale * g;
      h = h - f * g;
      d[i-1] = f - g;
      for (int j = 0; j < i; j++) {
        e[j] = 0.0f;
      }

      // Apply similarity transformation to remaining columns.

      for (int j = 0; j < i; j++) {
        f = d[j];
        V[j][i] = f;
        g = e[j] + V[j][j] * f;
        for (int k = j+1; k <= i-1; k++) {
          g += V[k][j] * d[k];
          e[k] += V[k][j] * f;
        }
        e[j] = g;
      }
      f = 0.0f;
      for (int j = 0; j < i; j++) {
        e[j] /= h;
        f += e[j] * d[j];
      }
      float hh = f / (h + h);
      for (int j = 0; j < i; j++) {
        e[j] -= hh * d[j];
      }
      for (int j = 0; j < i; j++) {
        f = d[j];
        g = e[j];
        for (int k = j; k <= i-1; k++) {
          V[k][j] -= (f * e[k] + g * d[k]);
        }
        d[j] = V[i-1][j];
        V[i][j] = 0.0f;
      }
    }
    d[i] = h;
  }

  // Accumulate transformations.

  for (volatile int i = 0; i < SIZE-1; i++) {
    V[SIZE-1][i] = V[i][i];
    V[i][i] = 1.0f;
    float h = d[i+1];
    if (h != 0.0f) {
      for (int k = 0; k <= i; k++) {
        d[k] = V[k][i+1] / h;
      }
      for (int j = 0; j <= i; j++) {
        float g = 0.0f;
        for (int k = 0; k <= i; k++) {
          g += V[k][i+1] * V[k][j];
        }
        for (int k = 0; k <= i; k++) {
          V[k][j] -= g * d[k];
        }
      }
    }
    for (int k = 0; k <= i; k++) {
      V[k][i+1] = 0.0f;
    }
  }
  for (int j = 0; j < SIZE; j++) {
    d[j] = V[SIZE-1][j];
    V[SIZE-1][j] = 0.0f;
  }
  V[SIZE-1][SIZE-1] = 1.0f;
  e[0] = 0.0f;
} 

// Symmetric tridiagonal QL algorithm.

void tql2(float V[SIZE][SIZE], float d[SIZE], float e[SIZE]) {

//  This is derived from the Algol procedures tql2, by
//  Bowdler, Martin, Reinsch, and Wilkinson, Handbook for
//  Auto. Comp., Vol.ii-Linear Algebra, and the corresponding
//  Fortran subroutine in EISPACK.

  for (int i = 1; i < SIZE; i++) {
    e[i-1] = e[i];
  }
  e[SIZE-1] = 0.0f;

  float f = 0.0f;
  float tst1 = 0.0f;
  float eps = pow(2.0f,-52.0f);
  for (int l = 0; l < SIZE; l++) {

    // Find small subdiagonal element

    tst1 = MAX(tst1,fabs(d[l]) + fabs(e[l]));
    int m = l;
    while (m < SIZE) {
      if (fabs(e[m]) <= eps*tst1) {
        break;
      }
      m++;
    }

    // If m == l, d[l] is an eigenvalue,
    // otherwise, iterate.

    if (m > l) {
      int iter = 0;
      do {
        iter = iter + 1;  // (Could check iteration count here.)

        // Compute implicit shift

        float g = d[l];
        float p = (d[l+1] - g) / (2.0f * e[l]);
        float r = hypot2(p,1.0f);
        if (p < 0) {
          r = -r;
        }
        d[l] = e[l] / (p + r);
        d[l+1] = e[l] * (p + r);
        float dl1 = d[l+1];
        float h = g - d[l];
        for (int i = l+2; i < SIZE; i++) {
          d[i] -= h;
        }
        f = f + h;

        // Implicit QL transformation.

        p = d[m];
        float c = 1.0f;
        float c2 = c;
        float c3 = c;
        float el1 = e[l+1];
        float s = 0.0f;
        float s2 = 0.0f;
        for (int i = m-1; i >= l; i--) {
          c3 = c2;
          c2 = c;
          s2 = s;
          g = c * e[i];
          h = c * p;
          r = hypot2(p,e[i]);
          e[i+1] = s * r;
          s = e[i] / r;
          c = p / r;
          p = c * d[i] - s * g;
          d[i+1] = h + s * (c * g + s * d[i]);

          // Accumulate transformation.

          for (int k = 0; k < SIZE; k++) {
            h = V[k][i+1];
            V[k][i+1] = s * V[k][i] + c * h;
            V[k][i] = c * V[k][i] - s * h;
          }
        }
        p = -s * s2 * c3 * el1 * e[l] / dl1;
        e[l] = s * p;
        d[l] = c * p;

        // Check for convergence.

      } while (fabs(e[l]) > eps*tst1);
    }
    d[l] = d[l] + f;
    e[l] = 0.0f;
  }
  
  // Sort eigenvalues and corresponding vectors.

  for (int i = 0; i < SIZE-1; i++) {
    int k = i;
    float p = d[i];
    for (int j = i+1; j < SIZE; j++) {
      if (fabs(d[j]) < fabs(p)) {
        k = j;
        p = d[j];
      }
    }
    if (k != i) {
      d[k] = d[i];
      d[i] = p;
      for (int j = 0; j < SIZE; j++) {
        p = V[j][i];
        V[j][i] = V[j][k];
        V[j][k] = p;
      }
    }
  }
}

void eigen_decomposition(float A[SIZE][SIZE], float V[SIZE][SIZE], float d[SIZE]) {
  float e[SIZE];
  for (int i = 0; i < SIZE; i++) {
    for (int j = 0; j < SIZE; j++) {
      V[i][j] = A[i][j];
    }
  }
  tred2(V, d, e);
  tql2(V, d, e);
}
