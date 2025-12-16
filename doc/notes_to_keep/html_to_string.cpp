#include <iostream>
#include <fstream> 

#define BUFFER_SIZE 5000

//void htmlToString(std::string filename, std::string &htmlcontent)
/*std::string htmlToString(std::string filename)
{
	std::fstream filestream;//in HTTP request?
	std::string	content; //in object request?? / socket?
	char buffer[BUFFER_SIZE];//in function

	filestream.open(filename, std::fstream::in);
	//if open doesnt work -> send 404 error

	filestream.read(buffer, BUFFER_SIZE);//change size
	//here we can change the size if the buffer for non blocking. 
	// somewhere it has to check if it is the end of the html file
	content = buffer;
	filestream.close();
	return (content);
}*/

/*int main(void)
{
	std::string content;
	content = htmlToString("roofs.html");
	std::cout << content << std::endl;
	return (0);
}*/

/*
senderror404()
{

}

bool forLosers(std::string filename, std::string s1, std::string s2)
{
	std::string newname = filename + ".replace";
	std::string line;

	std::ifstream myinfile(filename.c_str());
	if(myinfile.is_open() == false)
	{
		std::cout << "problem to open the file" << std::endl;
		return (false);
	}
	std::ofstream myoutfile(newname.c_str());
	if (myoutfile.is_open() == false)
	{
		std::cout << "problem to create the file" << std::endl;
		myinfile.close();
		return (false);
	}
	while(getline(myinfile, line))
		myoutfile << changeLine(line, s1, s2) << std::endl;
	myinfile.close();
	myoutfile.close();
	return (true);
}*/