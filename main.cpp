#include <iostream>
#include "Image.hpp"

int main()
{
	Image img("test_img/che.jpg");
	img.write("test_img/resnew.jpg");
	Image another = img;
	for (int i = 0; i < another.w * another.channels; ++i)
		another.data[i] = 123;
	another.write("test_img/resanother.jpg");
	Image img2(100, 100, 3);
	img2.write("test_img/resimg2.jpg");
}
