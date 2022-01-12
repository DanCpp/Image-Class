#include <iostream>
#include "Image.hpp"

int main()
{
	Image img("test_img/che.jpg");
	img.encodemessage("Message");
	img.write("test_img/hidemsg.jpg");
	char buffer[256] = {0};
	size_t size = 0;
	img.decodemessage(buffer, &size);
	std::cout << buffer << std::endl << size << std::endl;
}
