#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <iostream> 
#include <string>
using namespace std;

#ifdef WIN32
#include <windows.h>
#define DECODER_LIB TEXT( "D:\\Decoder\\x64\\Release\\RecaserLib.dll" )
#else
#include <dlfcn.h>
#define DECODER_LIB "libRecaserLib.so"
#endif

typedef void* (*__pInit   )( const char* );

typedef char* (*__pDo_job )( void*, const char*, int, const char* );

typedef void  (*__pDestroy)( void* );

int main( int argc, char * argv[] )
{  
    if( argc < 3 )
    {
        cerr<<"[USAGE] EXE CONFIG TEST\n"<<flush;
        exit( 1 );
    }

#ifdef WIN32
    HINSTANCE hInsDll = ::LoadLibrary( DECODER_LIB );
    if( hInsDll == NULL )
    {
        ::FreeLibrary( hInsDll );
        cerr<<"Error!\n";
        exit( 1 );
    }

    __pInit __init = ( __pInit )::GetProcAddress( hInsDll, "__init" );
    if( __init == NULL )
    {
        ::FreeLibrary( hInsDll );
        cerr<<"Error2!\n";
        exit( 1 );
    } 

    __pDo_job __do_job = ( __pDo_job )::GetProcAddress( hInsDll, "__do_job" );
    if( __do_job == NULL )
    {
        ::FreeLibrary( hInsDll );
        cerr<<"Error3!\n";
        exit( 1 );
    } 

    __pDestroy __destroy = ( __pDestroy )::GetProcAddress( hInsDll, "__destroy" );
    if( __destroy == NULL )
    {
        ::FreeLibrary( hInsDll );
        cerr<<"Error4!\n";
        exit( 1 );
    }
#else
    void* handle = dlopen( DECODER_LIB, RTLD_LAZY );
    if( handle == NULL )
    {
        cerr<<"Error!\n";
        exit( 1 );
    }

    dlerror();
    __pInit __init = ( __pInit ) dlsym( handle, "__init" );
    __pDo_job __do_job = ( __pDo_job ) dlsym( handle, "__do_job" );
    __pDestroy __destroy = ( __pDestroy ) dlsym( handle, "__destroy" );

    const char* dlsym_error = dlerror();
    if( dlsym_error )
    {
        cerr<<"Error2!\n";
        exit( 1 );
    }
#endif

    string config( argv[ 1 ] );
    void* class_handle = ( void* )__init( config.c_str() );

    cerr<<argv[ 2 ]<<"\n"<<flush;
    ifstream infile( argv[ 2 ] );
    if( !infile )
    {
        cerr<<"Can not open file "<<argv[ 2 ]<<"\n"<<flush;
        exit( 1 );
    }

    string sentence;
    size_t lineNo = 0;
    while( getline( infile, sentence ) )
    {
        ++lineNo;
        char * output = __do_job( class_handle, sentence.c_str(), 0, sentence.c_str() );
        delete []output;
    }

    infile.close();
    __destroy( class_handle );

#ifdef WIN32
    ::FreeLibrary( hInsDll );
#else
    dlclose( handle );
#endif

    return 0;
}  
