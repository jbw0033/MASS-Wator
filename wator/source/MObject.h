#ifndef MASSOBJECT_H
#define MASSOBJECT_H

class MObject {
 public:
  MObject( ) { };
  virtual void *callMethod( int functionId, void *argument ) = 0;
  virtual ~MObject( ) { };
};

typedef MObject *instantiate_t( void *argument );
typedef void destroy_t( MObject * );

#endif
