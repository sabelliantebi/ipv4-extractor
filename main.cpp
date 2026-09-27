#include <iostream>
#include <string>
#include <cctype>

//Returns true if c can be part of a token (digit, '.' or ':')
static bool isTokenChar(char c) //checks if a character can be part of an ipv4 token
{
    return std::isdigit(static_cast<unsigned char>(c)) || c == '.' || c == ':';//returns true if the character is a number, dot, or colon, unsigned char cast: isdigit needs a positive value, special characters like ñ can be negative

}

// Reads a number from tok starting at pos.
// maxDigits: max number of digits allowed
// maxValue:  max value allowed
// On success, pos moves after the digits and value has the number.
static bool readNumber(const std::string& tok, size_t& pos, int maxDigits, unsigned long maxValue, unsigned long& value) //reads a number from the token starting at the current position
{
    // First count the digits (before any math, so no overflow)
    size_t start = pos; //saves where the number starts
    size_t end = pos; //creates a variable to find where the number ends
    while (end < tok.size() && std::isdigit(static_cast<unsigned char>(tok[end]))) //keeps moving while the characters are digits, 
        end++;

    int count = static_cast<int>(end - start); //calculates how many digits were found
    if (count < 1 || count > maxDigits) //checks if there are no digits or too many digits
        return false;

    // No leading zero, only "0" alone is ok
    if (count > 1 && tok[start] == '0') //does not allow leading zeros like 001
        return false;

    // Now it is safe to convert by hand
    value = 0; //starts the numeric value at 0
    for (size_t i = start; i < end; i++) //goes through every digit
        value = value * 10 + (tok[i] - '0'); //moves the number one place left and adds the new digit ('7' - '0' = 7)

    if (value > maxValue) //checks if the number is bigger than the allowed maximum
        return false;

    pos = end; //moves the position after the number
    return true;
}

// Checks one full token. It must be A.B.C.D or A.B.C.D:PORT, nothing more.
static bool checkToken(const std::string& tok, unsigned long& address, int& port) //checks if a complete token is a valid ipv4 address
{
    size_t pos = 0; //starts reading at the beginning of the token
    unsigned long octet = 0; //stores one ipv4 octet
    unsigned long result = 0; //stores the complete ipv4 address as one number

    for (int i = 0; i < 4; i++) //loops four times for the four ipv4 octets
    {
        if (!readNumber(tok, pos, 3, 255, octet)) //reads one octet with maximum 3 digits and value 255
            return false;
        result = result * 256 + octet; //moves the previous octets 8 bits left and adds the new octet

        if (i < 3) //checks if this is not the last octet
        {
            if (pos >= tok.size() || tok[pos] != '.') //checks that there is a dot after the octet
                return false;
            pos++; // skip '.'
        }
    }

    // After the 4 octets: end of token, or ':' and a port
    if (pos == tok.size()) //checks if the token ends after the ipv4 address
    {
        address = result; //stores the valid ipv4 address
        port = -1; //uses -1 because there is no port
        return true; //returns true because the ipv4 is valid
    }

    if (tok[pos] != ':') //if the next character is not a colon, the token is invalid
        return false;
    pos++; //skips the colon

    unsigned long p = 0; //creates a variable to store the port
    if (!readNumber(tok, pos, 5, 65535, p)) //reads the port with maximum 5 digits and value 65535
        return false;

    // The port must be the last thing in the token
    if (pos != tok.size()) //checks that nothing comes after the port
        return false;

    address = result; //stores the complete ipv4 address
    port = static_cast<int>(p); //converts and stores the port as an integer
    return true; //returns true because the ipv4 and port are valid
}

bool extractIPv4(const std::string& str, unsigned long& outAddress, int& outPort) //searches a string and extracts a valid ipv4 address
{
    outAddress = 0; //starts the output address at 0
    outPort = -1; //starts the port at -1 meaning no port

    size_t i = 0; //starts reading from the beginning of the string
    while (i < str.size()) //keeps checking until the end of the string
    {
        // Skip garbage
        if (!isTokenChar(str[i])) //checks if the current character cannot be part of an ipv4
        {
            i++; //moves to the next character
            continue; //continues searching
        }

        // Take the whole token
        size_t start = i; //saves where the possible ipv4 token starts
        while (i < str.size() && isTokenChar(str[i])) //moves while the characters can be part of an ipv4 token
            i++;
        std::string tok = str.substr(start, i - start); //extracts the possible ipv4 token from the string

        unsigned long address = 0; //creates a temporary variable for the ipv4 address
        int port = -1; //creates a temp variable for the port
        if (checkToken(tok, address, port)) //checks if the extracted token is a valid ipv4 address
        {
            outAddress = address; //stores the valid ipv4 address
            outPort = port; //stores the port
            return true; //returns true because a valid ipv4 was found
        }
    }

    return false; //returns false if no valid ipv4 address was found
}

int main()
{
    std::string line; //creates a string for the user's input

    while (true) //keeps running until the user enters END
    {
        std::cout << "Enter a string (or 'END' to quit): "; //asks the user to enter a string
        if (!std::getline(std::cin, line)) //reads the complete line entered by the user
            break;

        if (line == "END") //checks if the user wants to end the program
        {
            std::cout << "Program terminated." << std::endl; //prints the termination message
            break; //ends the loop
        }

        unsigned long address = 0; //creates a variable to store the ipv4 address
        int port = -1; //creates a variable to store the port

        if (extractIPv4(line, address, port)) //tries to extract a valid ipv4 address from the input
        {
            std::cout << "Extracted IPv4 address: " 
                << ((address >> 24) & 255) << "." //prints the first ipv4 octet
                << ((address >> 16) & 255) << "." //prints the second ipv4 octet
                << ((address >> 8) & 255) << "."  //prints the third ipv4 octet
                << (address & 255) //prints the fourth ipv4 octet
                << " (decimal value: " << address << ", port: "; //prints the ipv4 address as a decimal value
            if (port == -1) //checks if the ipv4 does not have a port
                std::cout << "none"; //prints none when there is no port
            else
                std::cout << port; //prints the port number
            std::cout << ")" << std::endl; //closes the output and moves to a new line
        }
        else
        {
            std::cout << "Invalid input: no valid IPv4 address found" << std::endl; //prints an error if no valid ipv4 was found
        }
    }

    return 0; //ends the program successfully
}