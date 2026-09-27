#include <iostream>
#include <string>
#include <cctype>

// Returns true if c can be part of a token (digit, '.' or ':')
static bool isTokenChar(char c)
{
    return std::isdigit(static_cast<unsigned char>(c)) || c == '.' || c == ':';
}

// Reads a number from tok starting at pos.
// maxDigits: max number of digits allowed
// maxValue:  max value allowed
// On success, pos moves after the digits and value has the number.
static bool readNumber(const std::string& tok, size_t& pos, int maxDigits, unsigned long maxValue, unsigned long& value)
{
    // First count the digits (before any math, so no overflow)
    size_t start = pos;
    size_t end = pos;
    while (end < tok.size() && std::isdigit(static_cast<unsigned char>(tok[end])))
        end++;

    int count = static_cast<int>(end - start);
    if (count < 1 || count > maxDigits)
        return false;

    // No leading zero, only "0" alone is ok
    if (count > 1 && tok[start] == '0')
        return false;

    // Now it is safe to convert by hand
    value = 0;
    for (size_t i = start; i < end; i++)
        value = value * 10 + (tok[i] - '0');

    if (value > maxValue)
        return false;

    pos = end;
    return true;
}

// Checks one full token. It must be A.B.C.D or A.B.C.D:PORT, nothing more.
static bool checkToken(const std::string& tok, unsigned long& address, int& port)
{
    size_t pos = 0;
    unsigned long octet = 0;
    unsigned long result = 0;

    for (int i = 0; i < 4; i++)
    {
        if (!readNumber(tok, pos, 3, 255, octet))
            return false;
        result = result * 256 + octet;

        if (i < 3)
        {
            if (pos >= tok.size() || tok[pos] != '.')
                return false;
            pos++; // skip '.'
        }
    }

    // After the 4 octets: end of token, or ':' and a port
    if (pos == tok.size())
    {
        address = result;
        port = -1;
        return true;
    }

    if (tok[pos] != ':')
        return false;
    pos++; // skip ':'

    unsigned long p = 0;
    if (!readNumber(tok, pos, 5, 65535, p))
        return false;

    // The port must be the last thing in the token
    if (pos != tok.size())
        return false;

    address = result;
    port = static_cast<int>(p);
    return true;
}

bool extractIPv4(const std::string& str, unsigned long& outAddress, int& outPort)
{
    outAddress = 0;
    outPort = -1;

    size_t i = 0;
    while (i < str.size())
    {
        // Skip garbage
        if (!isTokenChar(str[i]))
        {
            i++;
            continue;
        }

        // Take the whole token
        size_t start = i;
        while (i < str.size() && isTokenChar(str[i]))
            i++;
        std::string tok = str.substr(start, i - start);

        unsigned long address = 0;
        int port = -1;
        if (checkToken(tok, address, port))
        {
            outAddress = address;
            outPort = port;
            return true;
        }
    }

    return false;
}

int main()
{
    std::string line;

    while (true)
    {
        std::cout << "Enter a string (or 'END' to quit): ";
        if (!std::getline(std::cin, line))
            break;

        if (line == "END")
        {
            std::cout << "Program terminated." << std::endl;
            break;
        }

        unsigned long address = 0;
        int port = -1;

        if (extractIPv4(line, address, port))
        {
            std::cout << "Extracted IPv4 address: "
                << ((address >> 24) & 255) << "."
                << ((address >> 16) & 255) << "."
                << ((address >> 8) & 255) << "."
                << (address & 255)
                << " (decimal value: " << address << ", port: ";
            if (port == -1)
                std::cout << "none";
            else
                std::cout << port;
            std::cout << ")" << std::endl;
        }
        else
        {
            std::cout << "Invalid input: no valid IPv4 address found" << std::endl;
        }
    }

    return 0;
}