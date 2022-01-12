#include <iostream>
#include "Image.hpp"

int main()
{
	Image img("test_img/test.jpg");
	Image img2("test_img/test2.jpg");

	Image diff = img;
	diff.scale_diffmap(img2);
	diff.write("test_img/scldiff.jpg");
}
