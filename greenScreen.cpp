#include "greenScreen.h"
#include <memory>
#include "lib/stb_image_resize2.h"

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
    for (size_t i = 0; i < static_cast<size_t>( size * 3 ); i += 3)
    {
        int r = static_cast<int>(screenStream[i]);
        int g = static_cast<int>(screenStream[i + 1]);
        int b = static_cast<int>(screenStream[i + 2]);

        // Treat as green only when green is both bright and dominant over red/blue.
        if (g > 165 && g > r + 40 && g > b + 40)
        {
            res[i] = imgStream[i];
            res[i + 1] = imgStream[i + 1];
            res[i + 2] = imgStream[i + 2];
        }
        else
        {
            res[i] = screenStream[i];
            res[i + 1] = screenStream[i + 1];
            res[i + 2] = screenStream[i + 2];
        }
    }

    Image resImage{res, screen.getWidth(), screen.getHeight()};
    resImage.Save(name);
    std::cout<<"Executed Successfully, new image in: "<<name<<std::endl;
}
