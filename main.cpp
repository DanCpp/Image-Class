#include <iostream>
#include "Image.hpp"

int main()
{
	Image img("test_img/papich.jpg");
	double ker[] = { 1 / 16.0, 2 / 16.0, 1 / 16.0,
	2 / 16.0, 4 / 16.0, 2 / 16.0,
	1 / 16.0, 2 / 16.0, 1 / 16.0 };
	img.convole_cyclic(0, 3, 3, ker, 1, 1);
	img.convole_cyclic(1, 3, 3, ker, 1, 1);
	img.convole_cyclic(2, 3, 3, ker, 1, 1);

	img.write("test_img/blurredborder.jpg");
}
