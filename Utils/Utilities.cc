/*

    FileName :                     Utilities.cc
    Content :                      Some objects that might come in handy
    Programmer :                   Nicolas PIERRE
    Version :                      1.0
    Date of creation :             10/06/14
    Support :                      mail to : nicolas.pierre@icloud.com

 */

#include "../Utils/Utilities.h"

//template<typename ... Args>
//std::string string_format ( const std::string& format, Args ... args )
//{
//size_t size = snprintf ( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
//std::unique_ptr<char[]> buf ( new char[ size ] );
//snprintf ( buf.get(), size, format.c_str(), args ... );
//return std::string ( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
//}

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

std::string getDataFileName (const std::string& pPath, int& pRunNumber)
{

    std::string line;
    std::fstream cFile;
    std::string filename = expandEnvironmentVariables (pPath) + "/.run_number.txt";

    struct stat buffer;

    if (stat (filename.c_str(), &buffer) == 0)
    {

        cFile.open ( filename.c_str(), std::fstream::out | std::fstream::in );

        if ( cFile.is_open() )
        {
            cFile >> pRunNumber ;

            pRunNumber ++;
            cFile.clear();
            cFile.seekp ( 0 );
            cFile << pRunNumber;
            cFile.close();
        }
    }
    else if (pRunNumber != -1)
    {
        pRunNumber = 1;
        cFile.open (filename, std::fstream::out );
        cFile << pRunNumber;
        cFile.close();
    }

    //TString cRunString = Form ( "run_%04d.raw", pRunNumber );
    //return cRunString.Data();
    return string_format ("run_%04d.raw", pRunNumber);
}

std::string expandEnvironmentVariables ( std::string s )
{
    if ( s.find ( "${" ) == std::string::npos ) return s;

    std::string pre  = s.substr ( 0, s.find ( "${" ) );
    std::string post = s.substr ( s.find ( "${" ) + 2 );

    if ( post.find ( '}' ) == std::string::npos ) return s;

    std::string variable = post.substr ( 0, post.find ( '}' ) );
    std::string value    = "";

    post = post.substr ( post.find ( '}' ) + 1 );

    if ( getenv ( variable.c_str() ) != NULL ) value = std::string ( getenv ( variable.c_str() ) );

    return expandEnvironmentVariables ( pre + value + post );
}
