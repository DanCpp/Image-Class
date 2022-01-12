#include <iostream>
#include "Image.hpp"

int main()
{
	Image img("test_img/che.jpg");
	img.mask(1, 0, 1);
	img.write("test_img/maskche.jpg");
}
