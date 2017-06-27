/*

    FileName :                     Utilities.cc
    Content :                      Some objects that might come in handy
    Programmer :                   Nicolas PIERRE
    Version :                      1.0
    Date of creation :             10/06/14
    Support :                      mail to : nicolas.pierre@icloud.com

 */

#include "../Utils/Utilities.h"



//Get time took

long getTimeTook ( struct timeval& pStart, bool pMili )
{
    struct timeval end;
    long seconds ( 0 ), useconds ( 0 );

    gettimeofday ( &end, 0 );
    seconds = end.tv_sec - pStart.tv_sec;
    useconds = end.tv_usec - pStart.tv_usec;

    if ( pMili )
        return ( long ) ( seconds * 1e3 + useconds / 1000 );

    else
        return ( long ) ( seconds * 1e6 + useconds );
}

//--------------------------------------------------------------------------
//Press enter function

void myflush ( std::istream& in )
{
    in.ignore ( std::numeric_limits<std::streamsize>::max(), '\n' );
    in.clear();
}

void mypause()
{
    std::cout << "Press [Enter] to continue ...";
    std::cin.get();
}




const std::string currentDateTime()
{
    time_t now = time ( 0 );
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime ( &now );
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime ( buf, sizeof ( buf ), "_%d-%m-%y_%H:%M", &tstruct );

    return buf;
}

double MyErf ( double* x, double* par )
{
    double x0 = par[0];
    double width = par[1];
    double fitval ( 0 );

    // if ( x[0] < x0 ) fitval = 0.5 * TMath::Erfc( ( x0 - x[0] ) / width );
    // else fitval = 0.5 + 0.5 * TMath::Erf( ( x[0] - x0 ) / width );
    if ( x[0] < x0 ) fitval = 0.5 * erfc ( ( x0 - x[0] ) / (sqrt (2.) * width ) );
    else fitval = 0.5 + 0.5 * erf ( ( x[0] - x0 ) / (sqrt (2.) * width ) );

    return fitval;
}

uint32_t convertAnyInt ( const char* pRegValue )
{
    if ( std::string ( pRegValue ).find ( "0x" ) != std::string::npos ) return static_cast<uint32_t> ( strtoul ( pRegValue, 0, 16 ) );
    else if ( std::string ( pRegValue ).find ( "0b" ) != std::string::npos ) //return static_cast<uint32_t> ( strtoul ( pRegValue, 0, 16 ) );
    {
        std::bitset<32> cBitset (std::string (pRegValue).erase (0, 2) );
        return static_cast<uint32_t> (cBitset.to_ulong () );
    }
    else return static_cast<uint32_t> ( strtoul ( pRegValue, 0, 10 ) );

}

//uint16_t convertAnyInt ( const char* pRegValue )
//{
//if ( std::string ( pRegValue ).find ( "0x" ) != std::string::npos ) return static_cast<uint16_t> ( strtoul ( pRegValue, 0, 16 ) );
//else if ( std::string ( pRegValue ).find ( "0b" ) != std::string::npos ) //return static_cast<uint32_t> ( strtoul ( pRegValue, 0, 16 ) );
//{
//std::bitset<16> cBitset (std::string (pRegValue).erase (0, 2) );
//return static_cast<uint16_t> (cBitset.to_ulong () );
//}
//else return static_cast<uint16_t> ( strtoul ( pRegValue, 0, 10 ) );

//}


//uint8_t convertAnyInt ( const char* pRegValue )
//{
//if ( std::string ( pRegValue ).find ( "0x" ) != std::string::npos ) return static_cast<uint8_t> ( strtoul ( pRegValue, 0, 16 ) );
//else if ( std::string ( pRegValue ).find ( "0b" ) != std::string::npos ) //return static_cast<uint32_t> ( strtoul ( pRegValue, 0, 16 ) );
//{
//std::bitset<8> cBitset (std::string (pRegValue).erase (0, 2) );
//return static_cast<uint8_t> (cBitset.to_ulong () );
//}
//else return static_cast<uint8_t> ( strtoul ( pRegValue, 0, 10 ) );

//}

uint8_t reverseBits (uint8_t cValue)
{
    cValue = (cValue & 0xF0) >> 4 | (cValue & 0x0F) << 4;
    cValue = (cValue & 0xCC) >> 2 | (cValue & 0x33) << 2;
    cValue = (cValue & 0xAA) >> 1 | (cValue & 0x55) << 1;
    return cValue;
}


//
void tokenize ( const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters )
{
    std::vector<std::string> cTokens;
    cTokens.clear();

    // Skip delimiters at beginning.
    std::string::size_type lastPos = str.find_first_not_of ( delimiters, 0 );

    // Find first "non-delimiter".
    std::string::size_type pos = str.find_first_of ( delimiters, lastPos );

    while ( std::string::npos != pos || std::string::npos != lastPos )
    {
        // Found a token, add it to the vector.
        cTokens.push_back ( str.substr ( lastPos, pos - lastPos ) );

        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of ( delimiters, pos );

        // Find next "non-delimiter"
        pos = str.find_first_of ( delimiters, lastPos );
    }

    tokens = cTokens;
}

