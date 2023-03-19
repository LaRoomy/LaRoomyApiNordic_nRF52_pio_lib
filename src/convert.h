#ifndef CONVERT_H_
#define CONVERT_H_

class Convert {
public:
    /**
     * @brief Converts a unsigned 8-bit value to a hexadecimal string with 2 characters
     * 
     * @param toConvert The value to convert
     * @param buffer_out The buffer to receive the string (the buffer must be at least [2] fields)
     */
    static void u8BitValueToHexTwoCharBuffer(uint8_t toConvert, char* buffer_out)
    {
        if(toConvert < 16){
            buffer_out[0] = '0';
            buffer_out[1] = uIntValToHexChar(toConvert);
        }
        else {
            unsigned int firstVal = (unsigned int) (toConvert / 16);
            buffer_out[0] = uIntValToHexChar(firstVal);
            unsigned int secondVal = (unsigned int) (toConvert - (firstVal * 16));
            buffer_out[1] = uIntValToHexChar(secondVal);
        }
    }

    /**
     * @brief Converts the characters of a hexadecimal string with 2 fields to an unsigned 8-bit value
     * 
     * @param left The left character of the hexadecimal string
     * @param right The right character of the hexadecimal string
     * @return (unsigned int) - The conversion result
     */
    static unsigned int x2CharHexValueToU8BitValue(char left, char right){
        unsigned int lft = hexCharToUIntVal(left);
        unsigned int rgt = hexCharToUIntVal(right);
        return (unsigned int)((lft*16) + rgt);
    }

    /**
     * @brief Converts a signed 16-bit integer to a hexadecimal string with 4 characters
     * 
     * @param toConvert The value to convert
     * @param buffer_out The buffer to receive the string (the buffer must be at least [4] fields)
     */
    static void s16BitValueToHex4CharBuffer(int16_t toConvert, char* buffer_out)
    {
        uint16_t v = static_cast<uint16_t>(toConvert);
        uint8_t l = (uint8_t)(v & 0x00FF);
        uint8_t h = (uint8_t)((v & 0xFF00) >> 8);
        char tc[2];
        u8BitValueToHexTwoCharBuffer(h, tc);
        buffer_out[0] = tc[0];
        buffer_out[1] = tc[1];
        u8BitValueToHexTwoCharBuffer(l, tc);
        buffer_out[2] = tc[0];
        buffer_out[3] = tc[1];
    }

    /**
     * @brief Converts a hexadecimal string to a signed 16-bit integer
     * 
     * @param hl The left character of the high-byte
     * @param hr The right character of the high-byte
     * @param ll The left character of the low-byte
     * @param lr The right character of the low-byte
     * @return int16_t - The conversion result
     */
    static int16_t x4CharHexValueToSigned16BitValue(char hl, char hr, char ll, char lr){
        auto highLeft = hexCharToUIntVal(hl);
        auto highRight = hexCharToUIntVal(hr);
        auto lowLeft = hexCharToUIntVal(ll);
        auto lowRight = hexCharToUIntVal(lr);
        uint16_t result = (highLeft << 12) | (highRight << 8) | (lowLeft << 4) | (lowRight);
        return static_cast<int16_t>(result);
    }

    /**
     * @brief Converts a 4 character hexadecimal string to an unsigned 16-bit integer
     * 
     * @param hl The left character of the high-byte
     * @param hr The right character of the high-byte
     * @param ll The left character of the low-byte
     * @param lr The right character of the low-byte
     * @return uint16_t - The conversion result
     */
    static uint16_t x4CharHexValueToUnsigned16BitValue(char hl, char hr, char ll, char lr){
        auto highLeft = hexCharToUIntVal(hl);
        auto highRight = hexCharToUIntVal(hr);
        auto lowLeft = hexCharToUIntVal(ll);
        auto lowRight = hexCharToUIntVal(lr);
        uint16_t result = (highLeft << 12) | (highRight << 8) | (lowLeft << 4) | (lowRight);
        return result;
    }

    /**
     * @brief Converts an unsigned single value to a hexadecimal character equivalent
     * 
     * @param val The value to convert (Only values from 0 to 15 are accepted)
     * @return char 
     */
    static char uIntValToHexChar(unsigned int val){
        switch(val){
            case 0:
                return '0';
            case 1:
                return '1';
            case 2:
                return '2';
            case 3:
                return '3';
            case 4:
                return '4';
            case 5:
                return '5';
            case 6:
                return '6';
            case 7:
                return '7';
            case 8:
                return '8';
            case 9:
                return '9';
            case 10:
                return 'a';
            case 11:
                return 'b';
            case 12:
                return 'c';
            case 13:
                return 'd';
            case 14:
                return 'e';
            case 15:
                return 'f';
            default:
                return 'f';		
        }
    }

    /**
     * @brief Converts a hexadecimal character to the unsigned integer equivalent
     * 
     * @param h The character to convert
     * @return unsigned int 
     */
    static unsigned int hexCharToUIntVal(char h){
        switch(h){
            case '0':
                return 0;
            case '1':
                return 1;
            case '2':
                return 2;
            case '3':
                return 3;
            case '4':
                return 4;
            case '5':
                return 5;
            case '6':
                return 6;
            case '7':
                return 7;
            case '8':
                return 8;
            case '9':
                return 9;
            case 'a':
                return 10;
            case 'b':
                return 11;
            case 'c':
                return 12;
            case 'd':
                return 13;
            case 'e':
                return 14;
            case 'f':
                return 15;
            default:
                return 0;
        }
    }

    /**
     * @brief Converts a decimal unsigned integer to a single character
     * 
     * @param num The value to convert (only figures lower than 10 are processed)
     * @return the equvalent character or if the value is higher than 9 >> 'X' will be returned
     */
    static char numToChar(unsigned int num)
    {
        switch(num)
        {
            case 0:
            return '0';
            case 1:
            return '1';
            case 2:
            return '2';
            case 3:
            return '3';
            case 4:
            return '4';
            case 5:
            return '5';
            case 6:
            return '6';
            case 7:
            return '7';
            case 8:
            return '8';
            case 9:
            return '9';
            default:
            return 'X';
        }
    }

    /**
     * @brief Converts a single character to the equivalent decimal number
     * 
     * @param l The character to convert (if the character is not a number, zero will be returned)
     * @return unsigned int 
     */
    static unsigned int charToNum(char l)
    {
        switch(l)
        {
            case '0':
            return 0;
            case '1':
            return 1;
            case '2':
            return 2;
            case '3':
            return 3;
            case '4':
            return 4;
            case '5':
            return 5;
            case '6':
            return 6;
            case '7':
            return 7;
            case '8':
            return 8;
            case '9':
            return 9;
            default:
            return 0;	
        }
    }

    /**
     * @brief Converts a wide string to a utf-8 based string
     * 
     * @param wStr The string to Convert. The string must be zero-terminated.
     * @return String - the result
     */
    static String escapeNonAsciiCharacters(const wchar_t* wStr){
        String resultString;

        if(wStr != nullptr){
            unsigned int c = 0;
            while(wStr[c] != L'\0'){
                if(wStr[c] < (wchar_t)127){
                    resultString += (char)wStr[c];
                }
                else {                    
                    // convert to escape sequence
                    char valBuffer[4];
                    char utf8Buffer[7];
                    
                    utf8Buffer[0] = '\\';
                    utf8Buffer[1] = 'u';
                    Convert::s16BitValueToHex4CharBuffer((int16_t)wStr[c], valBuffer);
                    utf8Buffer[2] = valBuffer[0];
                    utf8Buffer[3] = valBuffer[1];
                    utf8Buffer[4] = valBuffer[2];
                    utf8Buffer[5] = valBuffer[3];
                    utf8Buffer[6] = '\0';
                    resultString += utf8Buffer;
                }
                c++;
            }
        }
        return resultString;
    }
};

#endif