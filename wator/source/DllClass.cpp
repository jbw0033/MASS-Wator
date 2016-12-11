#include "DllClass.h"
#include "MASS_base.h"
#include <sstream> // ostringstream

//Used to toggle output for DllClass
const bool printOutput = false;
//const bool printOutput = true;

/**
 * 
 * @param className
 */
DllClass::DllClass( string className ) {
  // For debugging
  ostringstream convert;

  // Create "./className"
  int char_len = 2 + className.size( ) + 1;
  char dot_className[char_len];
  bzero( dot_className, char_len );
  strncpy( dot_className, "./", 2 );
  strncat( dot_className, className.c_str( ), className.size( ) );

  agents = new vector<Agent*>;

  // load a given class
  if ( ( stub = dlopen( dot_className, RTLD_LAZY ) ) == NULL ) {
    if(printOutput == true){
        convert.str( "" );
        convert << "class: " << dot_className << " not found" << endl;
        MASS_base::log( convert.str( ) );
    }
    exit( -1 );
  }

  // register the object instantiation/destroy functions
  instantiate = ( instantiate_t * )dlsym( stub, "instantiate" ); 
  destroy = ( destroy_t * )dlsym( stub, "destroy" );
}

