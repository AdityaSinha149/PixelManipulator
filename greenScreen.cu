#include "greenScreen.h"
#include "lib/stb_image_resize2.h"
#include <memory>
#include "cuda_runtime.h"
#include <chrono>

__global__ void doGreenScreen(unsigned char* d_resultant, unsigned char* d_screen, unsigned char* d_img, int size)
{
        int i = blockIdx.x * blockDim.x + threadIdx.x;

        if(i < size)
        {
                int r = static_cast<int>(d_screen[3 * i]);
                int g = static_cast<int>(d_screen[3 * i + 1]);
                int b = static_cast<int>(d_screen[3 * i + 2]);

                if (g > 165 && g > r + 40 && g > b + 40)
                {
                        d_resultant[3 * i] = d_img[3 * i];
                        d_resultant[3 * i + 1] = d_img[3 * i + 1];
                        d_resultant[3 * i + 2] = d_img[3 * i + 2];
                }

                else
                {
                        d_resultant[3 * i] = d_screen[3 * i];
                        d_resultant[3 * i + 1] = d_screen[3 * i + 1];
                        d_resultant[3 * i + 2] = d_screen[3 * i + 2];
                }
        }
}


bool greenScreenImage::checkAspectRatio(const Image &img1, const Image &img2)
{
        if (img1.getHeight() * img2.getWidth() == img2.getHeight() * img1.getWidth())
        {
                return true;
        }
        else
        {
                return false;
        }
        }

int greenScreenImage::checkSizes(const Image &img1, const Image &img2)
{

        if(checkAspectRatio(img1, img2) == false)
        {
                return -1;
        }
        else
        {
                if(img1.getHeight() == img2.getHeight())
                {
                        return 0; // same size
                }

                if(img1.getHeight() > img2.getHeight())
                {
                        return 1; // img1 is bigger
                }

                return 2; // img2 is bigger
        }

}

void greenScreenImage::resizeBigToSmall(Image &big, const Image &small)
{
        Image tmp{small.getWidth(), small.getHeight(), big.getChannel()};
        stbir_pixel_layout layout = (big.getChannel() == 4) ? STBIR_RGBA : STBIR_RGB;
        stbir_resize_uint8_linear(big.getRGBStream().data(), big.getWidth(), big.getHeight(), big.getWidth()*big.getChannel(),
                                        tmp.getRGBStream().data(), small.getWidth(), small.getHeight(), small.getWidth()*small.getChannel(),
                                        layout);
        big = tmp;
}

void greenScreenImage::applyGreenScreen(Image &screen, Image &img, std::string name)
{

        int check = checkSizes(screen, img);
        if(check == -1) return;
        else if(check == 1) resizeBigToSmall(screen, img);
        else if(check == 2) resizeBigToSmall(img, screen);

        std::vector<unsigned char> screenStream = screen.getRGBStream();
        std::vector<unsigned char> imgStream = img.getRGBStream();

        int size = screen.getWidth()*screen.getHeight();
        std::vector<unsigned char> res (size * 3);

        unsigned char* d_res;
        unsigned char* d_screen;
        unsigned char* d_img;

        cudaMalloc((void**)&d_res, (size * 3) * sizeof(unsigned char));
        cudaMalloc((void**)&d_screen, (size * 3) * sizeof(unsigned char)); 
        cudaMalloc((void**)&d_img, (size * 3) * sizeof(unsigned char)); 

        cudaMemcpy(d_screen, screenStream.data(), (size * 3) * sizeof(unsigned char), cudaMemcpyHostToDevice);     
        cudaMemcpy(d_img, imgStream.data(), (size * 3) * sizeof(unsigned char), cudaMemcpyHostToDevice);   

        int threads = 256;
        int blocks = (size + threads - 1) / threads;

        auto start = std::chrono::high_resolution_clock::now();

        doGreenScreen<<<blocks, threads>>>(d_res, d_screen, d_img, size);

        cudaDeviceSynchronize();
        
        cudaMemcpy(res.data(), d_res, (size * 3) * sizeof(unsigned char), cudaMemcpyDeviceToHost);

        Image resImage{res, screen.getWidth(), screen.getHeight()};

        cudaFree(d_res);
        cudaFree(d_screen);
        cudaFree(d_img);

        resImage.Save(name);
        std::cout<<"Executed Successfully, new image in: "<<name<<std::endl;
}
