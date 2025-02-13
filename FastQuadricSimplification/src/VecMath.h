#pragma once
#include <math.h>
//#define loopi(start_l,end_l,step_l) for ( int i=start_l;i<end_l;i+=step_l )
#define loopi(start_l,end_l) for ( int i=start_l;i<end_l;++i )
//#define loopj(start_l,end_l,step_l) for ( int j=start_l;j<end_l;j+=step_l )
#define loopj(start_l,end_l) for ( int j=start_l;j<end_l;++j )
//#define loopk(start_l,end_l,step_l) for ( int k=start_l;k<end_l;k+=step_l )
#define loopk(start_l,end_l) for ( int k=start_l;k<end_l;++k )
#define loopm(start_l,end_l) for ( int m=start_l;m<end_l;++m )
#define loopl(start_l,end_l) for ( int l=start_l;l<end_l;++l )
#define loopn(start_l,end_l) for ( int n=start_l;n<end_l;++n )
#define loop(var_l,start_l,end_l) for ( int var_l=start_l;var_l<end_l;++var_l )
#define loops(a_l,start_l,end_l,step_l) for ( a_l = start_l;a_l<end_l;a_l+=step_l )


#ifndef vec3i
struct vec3i { int x,y,z;};
#endif

struct vec3f
{
    double x, y, z;
 
    inline vec3f( void ) {}

    inline vec3f( const double X, const double Y, const double Z )
    { x = X; y = Y; z = Z; }

    inline vec3f operator + ( const vec3f& a ) const
    { return vec3f( x + a.x, y + a.y, z + a.z ); }

	inline vec3f operator += ( const vec3f& a ) const
    { return vec3f( x + a.x, y + a.y, z + a.z ); }

    inline vec3f operator * ( const double a ) const
    { return vec3f( x * a, y * a, z * a ); }

    inline vec3f operator * ( const vec3f a ) const
    { return vec3f( x * a.x, y * a.y, z * a.z ); }

    inline vec3f operator = ( const vec3f a ) 
    { x=a.x;y=a.y;z=a.z;return *this; }

    inline vec3f operator / ( const vec3f a ) const
    { return vec3f( x / a.x, y / a.y, z / a.z ); }

    inline vec3f operator - ( const vec3f& a ) const
    { return vec3f( x - a.x, y - a.y, z - a.z ); }

    inline vec3f operator / ( const double a ) const
    { return vec3f( x / a, y / a, z / a ); }

    inline double dot( const vec3f& a ) const
    { return a.x*x + a.y*y + a.z*z; }

    inline vec3f cross( const vec3f& a , const vec3f& b ) 
    { 
		x = a.y * b.z - a.z * b.y;
		y = a.z * b.x - a.x * b.z;
		z = a.x * b.y - a.y * b.x;
		return *this;
	}

    inline double angle( const vec3f& v ) 
    { 
		vec3f a = v , b = *this;
		double dot = v.x*x + v.y*y + v.z*z;
		double len = a.length() * b.length();
		if(len==0)len=0.00001f;
		double input = dot  / len;
		if (input<-1) input=-1;
		if (input>1) input=1;
		return (double) acos ( input );
	}

    inline double angle2( const vec3f& v , const vec3f& w ) 
    { 
		vec3f a = v , b= *this;
		double dot = a.x*b.x + a.y*b.y + a.z*b.z;
		double len = a.length() * b.length();
		if(len==0)len=1;

		vec3f plane; plane.cross( b,w ); 

		if ( plane.x * a.x + plane.y * a.y + plane.z * a.z > 0 )
			return (double) -acos ( dot  / len );

		return (double) acos ( dot  / len );
	}

    inline vec3f rot_x( double a ) 
    { 
		double yy = cos ( a ) * y + sin ( a ) * z;
		double zz = cos ( a ) * z - sin ( a ) * y;
		y = yy; z = zz;
		return *this;
	}
    inline vec3f rot_y( double a ) 
    { 
		double xx = cos ( -a ) * x + sin ( -a ) * z;
		double zz = cos ( -a ) * z - sin ( -a ) * x;
		x = xx; z = zz;
		return *this;
	}
    inline void clamp( double min, double max ) 
    {
		if (x<min) x=min;
		if (y<min) y=min;
		if (z<min) z=min;
		if (x>max) x=max;
		if (y>max) y=max;
		if (z>max) z=max;
	}
    inline vec3f rot_z( double a ) 
    { 
		double yy = cos ( a ) * y + sin ( a ) * x;
		double xx = cos ( a ) * x - sin ( a ) * y;
		y = yy; x = xx;
		return *this;
	}
    inline vec3f invert() 
	{
		x=-x;y=-y;z=-z;return *this;
	}
    inline vec3f frac() 
	{
		return vec3f(
			x-double(int(x)),
			y-double(int(y)),
			z-double(int(z))
			);
	}

    inline vec3f integer() 
	{
		return vec3f(
			double(int(x)),
			double(int(y)),
			double(int(z))
			);
	}

    inline double length() const
    { 
		return (double)sqrt(x*x + y*y + z*z); 
	}

    inline vec3f normalize( double desired_length = 1 )
    { 
		double square = sqrt(x*x + y*y + z*z);
		x/=square;y/=square;z/=square;

		return *this; 
	}
    static vec3f normalize( vec3f a );

	static void random_init();
	static double random_double();
	static vec3f random();

	static int random_number;

	double random_double_01(double a){
		double rnf=a*14.434252+a*364.2343+a*4213.45352+a*2341.43255+a*254341.43535+a*223454341.3523534245+23453.423412;
		int rni=((int)rnf)%100000;
		return double(rni)/(100000.0f-1.0f);
	}

	vec3f random01_fxyz(){
		x=(double)random_double_01(x);
		y=(double)random_double_01(y);
		z=(double)random_double_01(z);
		return *this;
	}
};
