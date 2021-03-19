//Free3D.h - all the 3D data structures and constants for rendering models
/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
*/

#ifndef _Free3D_H
#define _Free3D_H

#include <stdint.h> //for uint32_t, etc.
#include <stdlib.h>
#include <stdio.h>
#include <freethought.h>

#define PI 3.14159265359  //180 degrees
#define PI_DIV_2 1.570796327  //90 degrees
#define PI_DIV_4 0.785398163  //45 degrees
#define PI_DIV_180 0.017453293  //1 degree
#define INV_PI_DIV_2 0.636619772 //1/(0.5*PI)  2/PI 	

//#define MAX_RENDER_WIDTH 800
#define MAX_RENDER_HEIGHT 600  //todo remove the need for this constant

typedef uint8_t u8;
typedef int16_t i16;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
#if Linux
typedef int64_t i64;
typedef uint64_t u64;
#endif
#if WIN32
typedef __int64 i64;
typedef unsigned __int64 u64;
#endif

//POINT3D
typedef union {
	struct {
		float x,y,z;
	};
	struct {
		float i[3];
	};
} POINT3D;

//CLIPPOINT3D
//when polygons are clipped the texture map and clip flag information of their points needs to be stored
#define POLY_CLIP_RIGHT 0x01
#define POLY_CLIP_LEFT 0x02
#define POLY_CLIP_TOP 0x04
#define POLY_CLIP_BOTTOM 0x08
#define POLY_CLIP_FRONT 0x10
#define P_XCLIP	(POLY_CLIP_LEFT|POLY_CLIP_RIGHT)
#define P_YCLIP	(POLY_CLIP_TOP|POLY_CLIP_BOTTOM)
#define P_ZCLIP	(POLY_CLIP_FRONT)
#define P_CLIPFLAG (P_XCLIP|P_YCLIP|P_ZCLIP)
#define TOTAL_CLIP 0x1
#define PARTIAL_CLIP 0x2
#define NO_CLIP 0X4
typedef struct
{
	float x,y,z;
	float u,v;
	int axis;
	unsigned int color; 
	unsigned int Flag;
} CLIPPOINT3D;

//LINE
//possibly surface normal should be calculated at the time of being loaded
typedef struct
{
	u32 p[2];  //only 2 point lines
	u32 color[2];  //color of each point in line
} F3D_LINE;

#define F3D_POLYGON_USE_POLYGON_NORMAL	0x1	//use the polygon normal for lighting instead of point normals (used only in Gouraud shading), because shape is rectilinear and point normals would be wrong for 2 polygons at 90 degrees to each other that share a point
//POLYGON
//possibly surface normal should be calculated at the time of being loaded
typedef struct
{
	u32 flags; 
	u32 numpoint;  //having 4 point polygons helps to stop triangle rectangle artifacts/lines, speed in rendering, size of model files
	u32 color[4]; //for flat shading
  u32 axis;  //axis this polygon belongs to
	u32 p[4];  //only 3 or 4 point polygons
	float u[4];
	float v[4]; //each points u,v (tu,tv)
	POINT3D n; //surface normal (used for flat shade lighting) is precomputed when model is loaded and doesn't change
	POINT3D an; //surface normal transformed by model axes
	u32 ti;
  float transparency;  //transparency 0.0-1.0 added 10/27/05, 0=not transparent,1=completely transparent (invisible), add transparent color?
} POLYGON;

//AXIS3D - todo: probably should just be float m[3][3] or m[9]
typedef struct 
{
	float x[3];
	float y[3];
	float z[3];
} AXIS3D;  //should be MATRIX3D because could be confused with MODELAXIS

#define MAXSUPERAXES 20  //maximum number of superaxes- deprecated
typedef struct
{
//	float x[3];
//	float y[3];
//	float z[3];
	AXIS3D m; //axis matrix - should probably be MATRIX3D
	POINT3D loc; //location of axis
	i32 numsuperaxis; //number of super axes
	i32 *superaxis;  //this axis will be transformed (in order) by each of the axes listed here (a superaxis of the foot is the lower leg, a superaxis of the lower leg is the upper leg). 
	//Using superaxis instead of subaxis is easier because this axis needs to be processed first before applying superaxes- including axis[0]- the axis for the entire model, 
	//for multiaxis models all axes have the first superaxis equal to 0 
	//superaxis method: for all points, get the associated axis, transform the point by the axis and then by all super axes in order
	//subaxis method- this method is possible: for all axes, go though and transform the points of all subaxes: this requires a flag that is reset each time, so you know when the points of an axis have been transformed by a super axis- and not to apply the axis transform to the points of that axis again
	i32 numjointpoint; //number of joint points on this axis (ex: lower leg has two one with foot and one with upper leg)
	POINT3D *jp; //original joint point
	POINT3D *ajp; //transformed joint points
	i32 numpoint; //number of points on this axis 
	i32 *p; //list of index of each point in this axis (ex: 0,1,2,3,4,5). Added at load time. Makes transforming an axis faster than going through all points in model to transform only those on a single axis. Instead the points are retrieved from using the indices in the pointlist. This also means that getting the pointer to an axis only has to happen once when applying the axis matrix to all the points on an axis, as opposed to having to find the axis of each point to apply the axis matrix.
} MODELAXIS3D;


//MODELJOINT - joints on a model are necessary in order to move one axis to another axis so that their shared joint point locations are the same. 
//For example with a robot, an accelerometer gives the rotation for a body segment, but because two segments are connected by a joint, the rotation of one moves the connected segment.
//It's not enough to just make the axis that moves the other axis a superaxis, because although a subaxis would be rotated by the model matrix correctly, and the superaxis would then be rotated correctly too, 
//when the superaxis is rotated by it's own rotation matrix, the subaxis will not remain in alignment. 
//The axes-subaxes structure formed because to animate a multiaxis model it is easy to just rotate an upper leg, and then the foot and lower leg also share in the same rotation because the upper leg is a superaxis to both. 
//But because the accelerometer on each robot segment gives the rotation of each segment, the joints will not stay together as each rotation is applied to each segment. 
//So joints are necessary to move the joints back together after their rotation matrix has been applied. 
//todo: explore other possible solutions- in particular to remove/combine axes and joints (one idea was to make each axis a unique model, and then connect them through some other data structure/file, or perhaps the info could be within each model file)
typedef struct
{
	i32 a[2]; //two axes the joint pair is on
	i32 p[2]; //index to two joint points (joints points for each axis are in axis structure, and are rotated with axis)
} MODELJOINT;


//TEXTURE
#define TEXTURENAMESIZE 256  //this value is hard coded into the model file
typedef struct
{
u32 flags;  //texture flags
char name[TEXTURENAMESIZE]; //filename of texture
i32 curframe;  //current frame of texture avi
i32 frames; //number of frames in texture avi
FILE *tfptr; //pointer to texture avi file
BITMAPFILEHEADER *bmp; //pointer to start of bitmap file header
BITMAPINFOHEADER *bmpinfo; //pointer to start of bitmap info header
u8 *bmpdata; //pointer to bitmap data
} TEXTUREMAP;


//POLYLIST - is the list of polygon intersections on a scanline (horizontal line)
//todo: this needs to be re-examined and improved- not a polylist for each line, but only 1 at any time
//and one that grows dynamically (not limited to 30)
//todo: add error msgbox when number of points on polylist needs to be increased
#define MAX_POINTS_ON_SCANLINE 30 
typedef struct
{
	int numx;   //number of x values on this scanline
	int x[MAX_POINTS_ON_SCANLINE];  //x values of polygons
	float z[MAX_POINTS_ON_SCANLINE]; //z values of polygons
	float u[MAX_POINTS_ON_SCANLINE],v[MAX_POINTS_ON_SCANLINE];  //texture map values of polygons
	unsigned int c[MAX_POINTS_ON_SCANLINE];  //color of polygons
} POLYLIST;

//Transparency buffer
//Note that [40] means that only 40 polygons can be on any pixel
//transp being float may cause issues initializing to 0 with memset
//todo: add error msgbox when number of points in transparency buffer needs to be increased
#define MAX_POINTS_ON_POINT_TRANSPARENCY_BUFFER 40 
typedef struct TRANSPARENCY_BUFFER
{
int num;  //number of pixels so far
float z[MAX_POINTS_ON_POINT_TRANSPARENCY_BUFFER]; //z value of polygon pixel
unsigned int color[MAX_POINTS_ON_POINT_TRANSPARENCY_BUFFER]; //color of polygon pixel
float transparency[MAX_POINTS_ON_POINT_TRANSPARENCY_BUFFER]; //transparency value of polygon
int order[MAX_POINTS_ON_POINT_TRANSPARENCY_BUFFER]; //z order of pixels
} TRANSPARENCY_BUFFER;




//MODEL flags
#define MODEL_NORENDER						0x00000001
#define MODEL_VIDEOTEXTURE					0x00000002  //texture is a video
#define MODEL_VIDEOLOOP						0x00000004
#define MODEL_NOLIGHTING					0x00000008
//Shapes
//to store the underlying shape of the model if any (to retain the equation for the shape, for example an Ellipsoid)
#define MODEL_SHAPE_POLYHEDRON_6_SIDES		0x00000010  
#define MODEL_SHAPE_ELLIPSOID				0x00000020
#define MODEL_SHAPE_SEMI_ELLIPSOID			0x00000040
#define MODEL_SHAPE_CYLINDER				0x00000080
#define MODEL_SHAPE_SEMI_CYLINDER			0x00000100
typedef struct MODEL
{
u32 flags; //info flags, MODEL_NORENDER, etc.
char filename[2048]; //current path and filename of model
char name[255];  //Name of the model
union {
POINT3D location;   //location of model
POINT3D center; //center of shape to cut
};
POINT3D newloc; //new location of model
POINT3D v; //velocity (model can be moved by motion, or velocity)
union { 
POINT3D scale; //(width,height,depth) to store the initial dimensions of some prototype shapes like 6-sided polyhedrons, Ellipsoids, Ellipsoids, etc.- used for object clipping?
struct {
	float width;
	float height;
	float depth;
};
POINT3D radius; //for circles, ellipses, arcs
};
POINT3D MaxExtent; //maximum point in all 3 dimensions (MaxExtent.y is used for sorting cuts for CNC)
POINT3D MinExtent; //minimum point in all 3 dimensions 
float mass;
u32 numaxis;  //number of axes in model
u32 numjoint; //number of joint pairs in model
u32 numpoint;  //number of points in model
u32 numnormal;  //number of normals (usually same as numpoints, but = 0 for lines, and other models)
u32 numline;  //number of lines in model
u32 numpolygon;  //number of polygons in model
u32 numtexture;  //number of textures in model
MODELAXIS3D *axis;  //address of model axes (axis 0 is always the entire model)
MODELJOINT *joint; //joint pairs of model
POINT3D *point;  //address of model points with the axis each point belongs to
u32 *pointaxis; //array with the same index as *point, which has the axis number the point belongs to- note that by using this, no point can belong to two axes
POINT3D *apoint;  //address of points transformed by model axes- could be only POINT3D?
CLIPPOINT3D *cpoint;  //address of points after camera transform
POINT3D *normal;	//normal of the point, computed when the model is created to reflect the geometrical shape (ex: for a sphere x/r^2i + y/r^2j + z/r^2k), used for Gouraud light shading
POINT3D *anormal; //normal of points after transform by model axes- could be only POINT3D?
F3D_LINE *line; //address of lines in model
POLYGON *polygon;  //address of polygons in model   
POINT3D *axisangle;		//axis angles (x,y,z) for each axis in radians 
POINT3D *axisloc; //location of axis relative to model points
TEXTUREMAP *texture; //pointer to list of texture maps
POINT3D speed; //x,y=Milling speed, z= drill: descent rate or mill (units=mm/min,in/min)
POINT3D increment;  //amount of change in (z) after all cuts in a single (z) plane are done (mm or inch)
POINT3D rotspeed; //rotational speed (for any of 3 axes) (units=rpm)
struct MODEL *next;   //to next model in linked list - perhaps should be removed in favor of just using F3D_MODEL_LIST but is a little less processing when loading models
} MODEL;


#define MOTION_TIME_INST 0x1  //this inst is a time inst
//minst is 1 instruction in a motion, either an axis of a model, (the basic building block of motions)
//or an endtime for the following axis instructions
//example=  rotate axis 0 for 1 second to an end axis of 45 degrees
//a time instruction and 1 or more axis instructions define an instruction group
//the axes are the final axes values after the time duration
//each frame the axes are incremented, relative to the current time (etime-curtime)
//this way an instruction group 1s, axis 0 ends at 45 degrees
//is performed in how ever many frames = 1s, 
//the more frames the smoother the animation
//09-24-03 had to change axes to angles and translations because axes 
//cannot be added or subtracted incrementaly, where angles can
typedef struct
{
u64 Bits; //MOTION_TIME_INST
u64 etime;  //or endtime in ns
i64 naxis;  //number of axis in model (i64 axes? should be i32)
POINT3D oa;  //original angles - stores model angles at last time inst (in radians)
POINT3D ot;  //original translation - stores model translation at last time inst
POINT3D a;  //angle - final angles for this time inst group (in radians)
POINT3D t;  //translation - final translation for this time inst group
//POINT3D ai;  //angle increment !field not part of motion file yet!
//POINT3D at;  //translation increment !field not part of motion file yet!
} MINST;


#define MOTION_LOOP 0x1
#define MOTION_PLAYING 0x2
//#define MOTION_SET_OAXIS 0x4  //set the original axes for the next motion inst group
//motion is a list of each set of instructions (in an array) that make 1 complete motion
//example= loop of 2 sets of instructions (set of instructions defined by time instruction)
//1st set moves model -45 degrees, 2nd set moves model 45 degrees
//structure of instance of motion
typedef struct MOTION
{  //may need to be 32 bit aligned?
u64 Bits; //MOTION_LOOP,MOTION_PLAYING
char name[255];  //motion filename *.mot
i64 curinst;  //index to first inst in current instruction group  [could make insts linked list, but this is easier]
i64 numinst;  //Number of instructions in this motion (includes all time and axis instructions)
u64 stime;   //starttime and 
u64 etime;  //endtime of current instruction group
char otype;// l,c, or m for camera, light or model - object motion is applied to 
char oname[255];// name of light, camera or model - object motion is applied to
MINST *inst;  //list of instructions
struct MOTION *next;  
} MOTION;

//structure of motion file header
typedef struct
{  //may need to be 32 bit aligned?
u64 Bits; //MOTION_LOOP,MOTION_PLAYING
char name[255];  //motion filename *.mot
i64 curinst;  //index to first inst in current instruction group  [could make insts linked list, but this is easier]
i64 numinst;  //Number of instructions in this motion (includes all time and axis instructions)
u64 stime;   //starttime and 
u64 etime;  //endtime of current instruction group
MINST *inst;  //list of instructions
MOTION *next;  
} MOTIONFILEHEADER;


//OVERLAY
//overlays are 2D rectangles that have no depth and are drawn on the final rendered image after rendering
//For example subtitles, text describing scale, etc.
#define OVERLAY_NORMAL					0x00000000
#define OVERLAY_FADEIN					0x00000001
#define OVERLAY_FADEOUT					0x00000002
#define OVERLAY_STOP_ON_NEXT_IMAGE      0x00000004
#define OVERLAY_INCLUDE_AUDIO           0x00000008
#define OVERLAY_IS_VIDEO				0x00000010	
#define OVERLAY_VIDEO_FILE_OPEN			0x00000020
#define OVERLAY_REPEAT					0x00000040
typedef struct
{
char name[255];  //name of overlay
u32 Bits; //
u8 *bmp;  //bitmap of overlay
int x,y,z;  //location of overlay
int w,h; //width and height
int cw,ch; //character width and height
//u32 textcolor;
//u32 bkcolor; //background of text color
//char text[1024]; //text on overlay
u32 effects; //OVERLAY_FADE_IN, OVERLAY_FADE_OUT, TRANSPARENT
float transparency; //how transparant
int transition; //how far into transition overlay is (ex: fade, etc) in ms
int duration; //duration of effect in ms
int EndTime; //End time in ms
char font[255];
//VIDEOFILE video;  //video structure
struct OVERLAY *next;
} OVERLAY;


//LIGHT
typedef struct LIGHT
{
	char name[255];
	char filename[2048]; //current path and filename of light file 
	unsigned int color;
	float deg; //degrees 0-6.28
	AXIS3D axis;  //orientation of light
	POINT3D location; //light (and axis) location
	POINT3D axisangle; //for motions to add angles
	i32 i; //intensity 0-100 - used? should be derived from distance
	struct LIGHT *next;
} LIGHT;

//list of models to exclude from rendering for a particular camera
typedef struct EXCLUDED
{
char ModelName[255]; //model name
char CameraName[255]; //camera name
struct EXCLUDED *next;
} EXCLUDED;


//CAMERA
#define CAMERA_RENDERED 0x00000001  //bmp was rendered for this frame, no need to render again
#define MAX_MODEL_NAME_SIZE	256 //note: old models have 255
typedef struct CAMERA
{
	u32 flags;
	char name[MAX_MODEL_NAME_SIZE];
	AXIS3D axis; //actual matrix
	AXIS3D oaxis; //actual matrix (original - used for motion)
	POINT3D location; //camera location - probably remove rloc or oloc once I revisit motions
	POINT3D rloc; //reference location for motions to move camera as angle grows
	POINT3D oloc; //original camera location at beginning of motion instruction
//	AXIS3D r; //reverse matrix for rendering - recalculated every time camera position or angle changes
	POINT3D axisangle; //for motions to add angles
	//POINT3D axisloc; 
	float focus; //focal length in pixels (default = 160.0)
	float trans_change;  //change in translation
	float rot_change;  //change in rotation
	u32 *bmp; //bitmap of image camera is capturing
	EXCLUDED *exclude; //list of excluded models - models this camera cannot see
	struct CAMERA *next;
} CAMERA;


//motionlist is a list contains the name of a motion 
//and the name of a model to apply the motion to
//a motion can be applying to any model 
//(presuming that the axes in the motion exist in the model)
//each frame (when motions are enabled) this list is gone thru 
//and all motions are incremented relative to the current time
//any motions that are complete are deleted from the list 
//with StopMotion(motionname,modelname) (EndMotion?)
//new motions are added to the end of the list 
//with PlayMotion(motionname,modelname) (StartMotion?)
//motion list contains all motions currently playing
typedef struct MOTIONLIST
{
char name[255]; //motion file name
MOTION *motion;
MODEL *model;
CAMERA *camera;
LIGHT *light;
struct MOTIONLIST *next;
} MOTIONLIST;

//A linked list that stores all the objects (camera,lights,models) for a scene
//one idea is view everything as a model, and then light or camera is a flag/property
typedef struct OBJLIST
{
char type; //type of object m,c,l
char name[255];  //name of obj - used only to delete object
//name is c#or l#for camera or light where #=number
//hopefully do away with name[255]
//i32 num; //camera or light number
MODEL *model;  //used for motions for faster access to model
CAMERA *camera;
LIGHT *light;
struct OBJLIST *prev;
struct OBJLIST *next;
} OBJLIST;

#define F3D_RENDER_NO_SCREEN_REFRESH			0x00000001 //do not refresh screen whenever RenderSceneFromCamera is called
#define F3D_RENDER_WHITE_BACKGROUND				0x00000002 //draw white background
#define F3D_RENDER_USE_TRANSPARENCY				0x00000004 //include transparency effects into the rendered image- USE_TRANSPARENCY_BUFFER
#define F3D_RENDER_USE_Z_BUFFER					0x00000008 //use a z buffer to store the order of polygon scanline z position
#define F3D_RENDER_CULL_BACKFACING_POLYGONS		0x00000010 //cull backface polygons
#define F3D_RENDER_USE_TEXTUREMAPS				0x00000020 //draw texturemaps on polygon surfaces if they exist
#define F3D_RENDER_USE_LIGHTING					0x00000040 //add effects of lights to polygons
//#define F3D_RENDER_NO_SHADING					0x00000080 //no shading of polygon faces- this is when USE_LIGHTING is not set
#define F3D_RENDER_FLAT_SHADING					0x00000080 //shading of polygon faces based on polygon vertex colors
#define F3D_RENDER_GOURAUD_SHADING				0x00000100 //gouraud shading of polygon faces
#define F3D_RENDER_POINTS_ONLY					0x00000200	//only render points of polygons
#define F3D_RENDER_LINES_ONLY					0x00000400	//only render lines of polygons
#define F3D_RENDER_POLYGON_FACES				0x00000800	//render polygon faces
#define F3D_RENDER_USE_TRANSPARENT_COLOR_KEY	0x00001000	//do not draw any point with the transparent (chroma) key color
#define F3D_RENDER_LIGHTING_HAS_NOT_CHANGED		0x00002000	//no need to calculate lighting at polygon vertices because none of the models or lights have moved since the last render (note cameras can move without this changing)
//#define F3D_RENDER_TEXTURES_LOADED				0x00002000	//texture map images have been loaded to the model memory
#define	F3D_RENDER_TRANSFORM_POINTS_BY_MODEL_AXES	0x00004000 //transform points by the model axes (for multi-axis models alt: MULTI_AXIS_MODELS)
#define	F3D_RENDER_PERSPECTIVE					0x00008000 //apply perspective transformation

#define F3D_RENDER_DEFAULT_TRANSPARENT_COLOR_KEY_COLOR 0xff00ff //default color to not render in texture maps
#define F3D_RENDER_POLYGON_CLIPPING			0x00010000 //clip polygons
typedef struct _F3D_Status
{
unsigned int flags;
unsigned int renderflags; //flags for rendering engine
OBJLIST *iobjlist; //pointer to object list - all objects in a scene
MODEL *imodel; //list of all loaded models this program is working with
CAMERA *icamera; //list of cameras
CAMERA *curcamera; //the current camera
LIGHT *ilight; //list of lights
int NumPolyhedron6Sides; //for giving 6-Sided Polyhedron models unique default names, incremented for each new 6-sided polyhedron made
int NumPolygon; //for unique name for model Polygons - could just use UniqueModelNum- but somewhat helps to track different models of the same shape
int NumEllipsoid; //for unique name for model Ellipsoids
int NumCylinder; //for unique name for model Cylinders
float tanFOV2x;  //tangent of the camera focal length and the screen's x
float tanFOV2y;	//tangent of the camera focal length and the screen's y
unsigned int PolyFlag; //stores the clip flags for a polygon
unsigned int LineFlag; //stores the clip flags for a line
CLIPPOINT3D clippoint[8];  //variable that temporarily holds polygon points being clipped
CLIPPOINT3D newpoint[8];  //variable that temporarily holds polygon points being clipped
POLYLIST pbuf[MAX_RENDER_HEIGHT];  //stores a polygon's x's on each horizontal line
float *zbuf; //pointer for z buffer float values [MAX_RENDER_WIDTH][MAX_RENDER_HEIGHT];
TRANSPARENCY_BUFFER *tbuf; //[MAX_RENDER_WIDTH][MAX_RENDER_HEIGHT];  //buffer that holds all the points on one pixel to determine final color due to partial transparency
unsigned int transparent_key_color; //transparent (chroma) color key color: the color not to render in texture maps
//unsigned int PolygonCount; //to determine the number of polygons per second being rendered
MODEL *CurrentModel; //currently selected (editing) model
LIGHT *CurrentLight; //currently selected (editing) light
char units[10]; //string of "mm" or "inches"
int DotsPerMM; //=100 (each CNC dot=0.01mm) 
int DotsPerInch; //=1000 (each CNC dot=0.001 inch) (1200dpi=0.000833 inch)
int WinMaxX,WinMaxY; //for ClipPoint to access render window max X and Y
//todo: change to just pointers:
//TRANSPARENCY_BUFFER* tbuf;
//float *zbuf;
#if Linux
	pthread_mutex_t	RenderLock; 
#endif
} F3D_Status;


#define NUMMODELFILESECTIONS	23
typedef enum
{
	MODFILE_MODEL_NAME                  = 0,    
	MODFILE_FLAGS,    
	MODFILE_LOCATION,    
	MODFILE_SCALE,
	MODFILE_SPEED,
	MODFILE_ROTSPEED,
	MODFILE_INCREMENT,
	MODFILE_MAXEXTENT,
	MODFILE_MINEXTENT,
	MODFILE_NUMAXES, 
	MODFILE_NUMJOINTS,
	MODFILE_NUMPOINTS,
	MODFILE_NUMNORMALS,
	MODFILE_NUMLINES,
	MODFILE_NUMPOLYGONS,
	MODFILE_NUMTEXTURES,
	MODFILE_AXES,
	MODFILE_JOINTS,
	MODFILE_POINTS,
	MODFILE_NORMALS,
	MODFILE_LINES,
	MODFILE_POLYGONS,
	MODFILE_TEXTURES
} MODEL_FILE_SECTIONS;


#define NUMPOLYGONSECTIONS	11
typedef enum
{
	MODFILE_POLYGON_POLYGON                  = 0,    
	MODFILE_POLYGON_FLAGS, 
	MODFILE_POLYGON_AXIS, 
	MODFILE_POLYGON_POINTS,
	MODFILE_POLYGON_U,
	MODFILE_POLYGON_V,
	MODFILE_POLYGON_COLOR,
	MODFILE_POLYGON_NORMAL,
	MODFILE_POLYGON_TEXTURE,
	MODFILE_POLYGON_TRANSPARENCY,
	MODFILE_POLYGON_END
} MODEL_FILE_POLYGON_SUBSECTIONS;

#define F3D_MAKE_NEW_CYLINDER_MAKE_TOP 1
#define F3D_MAKE_NEW_CYLINDER_MAKE_BOTTOM 2

//function prototypes
//int InitFree3D(void);
#if Linux
int _stricmp(char *str1, char *str2);
#endif
int F3DGetLineFromText(char *tline,char *buf);
int RotatePoints(POINT3D *points,POINT3D *angle,int numpoints);
int RotatePoint(POINT3D *points,POINT3D *angle);
int RotateAxes3D(AXIS3D *axes,POINT3D *angle,int numaxes);
int RotateAxis3D(AXIS3D *axis,POINT3D *angle);
void MakeIdentityMatrix(AXIS3D *axis);
void MakeRotationMatrix_AroundXAxis(AXIS3D *m,float a);
void MakeRotationMatrix_AroundYAxis(AXIS3D *m,float a);
void MakeRotationMatrix_AroundZAxis(AXIS3D *m,float a);
void PointsMatrix3DMultiply(POINT3D *p1,AXIS3D *axis,POINT3D *p2,int numpoints);
void PointMatrix3DMultiply(POINT3D *p1,AXIS3D *axis,POINT3D *p2);
void CalculatePolygonNormal(POLYGON *tpoly,POINT3D *tpoint);
void CalculateCrossProduct(POINT3D *v1,POINT3D *v2,POINT3D *v3);
void NormalizeVector(POINT3D *v);
void RenderSceneFromCamera(u8 *bmp,int w,int h,CAMERA *lcamera,int upsidedown);
void LightPolygon(MODEL *lmodel,POLYGON *tpolygon);
void DrawPolygon(u8 *bmp,int w, int h, CLIPPOINT3D *newpnt, POLYGON polygon, TEXTUREMAP *texture,int upsidedown);
void DrawLine(u8 *bmp,int w, int h, CLIPPOINT3D *newpnt, F3D_LINE line, TEXTUREMAP *texture,int upsidedown);
//void PointMatrix3DMultiplyAllAxes(MODEL *lmodel);
void TransformModelByAxes(MODEL *lmodel);
void PointsMatrix3DMultiplyToClipPoints(POINT3D *lpnt,AXIS3D axis,POINT3D axisloc,CLIPPOINT3D *newpnt, int numpnt);
unsigned int IsPointClipped(CLIPPOINT3D p);
unsigned int IsPointClippedPerspective(CLIPPOINT3D p);
int IsPolygonOrLineClipped(CLIPPOINT3D *lpnt,int nump,unsigned int *PolyFlag);
typedef float WithinFunc(int v);
int ClipToPlane(int oldn, WithinFunc *within);
int ClipLineToPlane(WithinFunc *within);
float WithinFront(int v);
float WithinBack(int v);
float WithinLeft(int v);
float WithinLeftFov(int v);
float WithinRight(int v);
float WithinRightFov(int v);
float WithinBottom(int v);
float WithinBottomFov(int v);
float WithinTop(int v);
float WithinTopFov(int v);
float WithinLeftRect(int v);
float WithinRightRect(int v);
float WithinBottomRect(int v);
float WithinTopRect(int v);
int ClipPolygon(int NumVertices);
int ClipLine();
void AddCameraToScene(CAMERA *lcamera); //Add camera to scene
void RemoveCameraFromScene(CAMERA *lcamera);  //Delete camera from scene
void RemoveAllCamerasFromScene(void);
void AddLightToScene(LIGHT *llight);  //Add light to scene
void RemoveLightFromScene(LIGHT *llight);  //Delete light from scene
void RemoveAllLightsFromScene(void);
int AddModelToModelList(MODEL *lmodel); //add a model to the list of all models the program is using
int RemoveModelFromModelList(MODEL *lmodel);  //remove a model from the list of models loaded (doesn't free space- for that use freemodel)
void AddModelToScene(MODEL *lmodel); //Add model to a scene
void RemoveModelFromScene(MODEL *lmodel); //remove a model from the scene (but don't unload)
void RemoveAllModelsFromScene(void); 
int FreeModel(MODEL *lmodel);
int FreeModelList(void);
int FreeLightList(void);
int FreeCameraList(void);
int FreeObjectList(OBJLIST **lobjlist);
MODEL* GetModel(char *name);  //get a pointer to a model
LIGHT* GetLight(char *name);  //get a pointer to a light
CAMERA* GetCameraByName(char *name);  //get a pointer to a camera by name
CAMERA* GetCameraByNum(int num);  //get a pointer to a camera by number (first camera = 1)
void MatrixMultiply3D(AXIS3D *m1,AXIS3D *m2,AXIS3D *m3);
int MakeModel_Polyhedron6Sides(MODEL *lmodel);
int MakeModel_XYZ_Axis(MODEL *lmodel);
int MakeModelEllipsoid(POINT3D scale,int numrows,int numcolumns,MODEL *lmodel,float transparency);
int MakeModelCylinder(POINT3D scale,int numcolumns,MODEL *lmodel,float transparency,unsigned int flags);
int SaveModel(MODEL *lmodel,char *filename);
MODEL* LoadModel(char *filename);
int F3DGetLineFromText(char *tline,char *buf);
int UnloadModel(MODEL *lmodel);
void LoadTexture(MODEL *lmodel,int ti,char *name);
unsigned char * F3DLoadBitmap(char *filename);
char *GetFileNameFromPath(char *filename);
int GetFileNameWithoutExtension(char *filename,char *dest);
int UpdateModelList(FTControl *tcontrol);
int InitFree3D(void);
int CloseFree3D(void);
#endif /* _Free3D_H */
