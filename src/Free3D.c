//Free3D.c - all the 3D functions for rendering models
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
#include "Free3D.h"
#if Linux
#include <pthread.h>
#endif

F3D_Status F3DStatus; //Free3D global variables (icamera,imodel,ilight, etc.)

#if Linux
int _stricmp(char *str1, char *str2) 
{
return(strcasecmp(str1,str2));
}
#endif

//Note that the order of rotation ix around x, around y, then around z because this makes a difference and must be presumed by the caller
//Note too that this is different from RotateModelPoints because there the model axis angles are updated
int RotatePoints(POINT3D *points,POINT3D *angle,int numpoints) 
{
	AXIS3D tmatrix;

	if (angle->x!=0.0) {  //rotate around x axis
		MakeRotationMatrix_AroundXAxis(&tmatrix,angle->x);  //convert angle to rotation matrix
		PointsMatrix3DMultiply(points,&tmatrix,points,numpoints);
	}
	if (angle->y!=0.0) {  //rotate around y axis
		MakeRotationMatrix_AroundYAxis(&tmatrix,angle->y);  //convert angle to rotation matrix
		PointsMatrix3DMultiply(points,&tmatrix,points,numpoints);
	}
	if (angle->z!=0.0) {  //rotate around z axis
		MakeRotationMatrix_AroundZAxis(&tmatrix,angle->z);  //convert angle to rotation matrix
		PointsMatrix3DMultiply(points,&tmatrix,points,numpoints);
	}

	return(1);
} //RotatePoints

//Note that the order of rotation ix around x, around y, then around z because this makes a difference and must be presumed by the caller
//Note too that this is different from RotateModelPoints because there the model axis angles are updated
int RotatePoint(POINT3D *point,POINT3D *angle) 
{
	AXIS3D tmatrix;

	if (angle->x!=0.0) {  //rotate around x axis
		MakeRotationMatrix_AroundXAxis(&tmatrix,angle->x);  //convert angle to rotation matrix
		PointMatrix3DMultiply(point,&tmatrix,point);
	}
	if (angle->y!=0.0) {  //rotate around y axis
		MakeRotationMatrix_AroundYAxis(&tmatrix,angle->y);  //convert angle to rotation matrix
		PointMatrix3DMultiply(point,&tmatrix,point);
	}
	if (angle->z!=0.0) {  //rotate around z axis
		MakeRotationMatrix_AroundZAxis(&tmatrix,angle->z);  //convert angle to rotation matrix
		PointMatrix3DMultiply(point,&tmatrix,point);
	}

	return(1);
} //RotatePoint


int RotateAxes3D(AXIS3D *axes,POINT3D *angle,int numaxes)
{
	AXIS3D tmatrix;
	int i;

	if (angle->x!=0.0) {  //rotate around x axis
		MakeRotationMatrix_AroundXAxis(&tmatrix,angle->x);  //convert angle to rotation matrix
		for(i=0;i<numaxes;i++) {
			MatrixMultiply3D(&axes[i],&tmatrix,&axes[i]);
		}
	}
	if (angle->y!=0.0) {  //rotate around y axis
		MakeRotationMatrix_AroundYAxis(&tmatrix,angle->y);  //convert angle to rotation matrix
		for(i=0;i<numaxes;i++) {
			MatrixMultiply3D(&axes[i],&tmatrix,&axes[i]);
		}
	}
	if (angle->z!=0.0) {  //rotate around z axis
		MakeRotationMatrix_AroundZAxis(&tmatrix,angle->z);  //convert angle to rotation matrix
		for(i=0;i<numaxes;i++) {
			MatrixMultiply3D(&axes[i],&tmatrix,&axes[i]);
		}
	}

	return(1);
} //int RotateAxes3D(int numaxes,MODELAXIS3D *axes,POINT3D *angle)

//rotate a single axis around 3 angles
int RotateAxis3D(AXIS3D *axis,POINT3D *angle)
{
	AXIS3D tmatrix;

	if (angle->x!=0.0) {  //rotate around x axis
		MakeRotationMatrix_AroundXAxis(&tmatrix,angle->x);  //convert angle to rotation matrix
		MatrixMultiply3D(axis,&tmatrix,axis);
	}
	if (angle->y!=0.0) {  //rotate around y axis
		MakeRotationMatrix_AroundYAxis(&tmatrix,angle->y);  //convert angle to rotation matrix
		MatrixMultiply3D(axis,&tmatrix,axis);	}
	if (angle->z!=0.0) {  //rotate around z axis
		MakeRotationMatrix_AroundZAxis(&tmatrix,angle->z);  //convert angle to rotation matrix
		MatrixMultiply3D(axis,&tmatrix,axis);
	}

	return(1);
} //int RotateAxis3D(MODELAXIS3D *axes,POINT3D *angle)



void MakeIdentityMatrix(AXIS3D *axis)
{
axis->x[0]=1.0;  axis->x[1]=0.0;  axis->x[2]=0.0;
axis->y[0]=0.0;  axis->y[1]=1.0;  axis->y[2]=0.0;
axis->z[0]=0.0;  axis->z[1]=0.0;  axis->z[2]=1.0;
}


void MakeRotationMatrix_AroundXAxis(AXIS3D *m,float a)
{
	//for some reason cos(0.0001) is returning 1.0000000
	//rotation matrix
	//left handed= c -s   right handed=  c s
	//             s  c                 -s c
m->x[0]=1.0;  m->x[1]=0.0;	m->x[2]=0.0;
m->y[0]=0.0;  m->y[1]=(float)cos(a);	m->y[2]=(float)-sin(a);
m->z[0]=0.0;  m->z[1]=(float)sin(a);	m->z[2]=(float)cos(a);
}


void MakeRotationMatrix_AroundYAxis(AXIS3D *m,float a)
{
	//rotation matrix
	//left handed= c  s   right handed=  c -s
	//            -s  c                  s  c
m->x[0]=(float)cos(a);	m->x[1]=0.0;	m->x[2]=(float)sin(a);
m->y[0]=0.0;		m->y[1]=1.0;	m->y[2]=0.0;
m->z[0]=(float)-sin(a);	m->z[1]=0.0;	m->z[2]=(float)cos(a);
}


void MakeRotationMatrix_AroundZAxis(AXIS3D *m,float a) 
{
	//rotation matrix
	//left handed= c -s   right handed=  c s
	//             s  c                 -s c
m->x[0]=(float)cos(a);		m->x[1]=(float)-sin(a);		m->x[2]=0.0;
m->y[0]=(float)sin(a);	m->y[1]=(float)cos(a);		m->y[2]=0.0;
m->z[0]=0.0;			m->z[1]=0.0;			m->z[2]=1.0;
}



//multiply a set of points by a 3x3 matrix
//p1 and p2 are each ararys of POINT3D and can be the same array
void PointsMatrix3DMultiply(POINT3D *p1,AXIS3D *axis,POINT3D *p2,int numpoints)
{
int i;
//if p2==p1 - just copying each component to p2 might lead to an error
POINT3D temp;

	for(i=0;i<numpoints;i++) {
		temp.x = p1[i].x*axis->x[0] + p1[i].y*axis->y[0] + p1[i].z*axis->z[0];
		temp.y = p1[i].x*axis->x[1] + p1[i].y*axis->y[1] + p1[i].z*axis->z[1];
		temp.z = p1[i].x*axis->x[2] + p1[i].y*axis->y[2] + p1[i].z*axis->z[2];	
		memcpy(&p2[i],&temp,sizeof(POINT3D));
	}
}  //void PointsMatrix3DMultiply(POINT3D *p1, AXIS3D *axis,POINT3D *p2, int numpoints)


//multiply a point by a 3x3 matrix
void PointMatrix3DMultiply(POINT3D *p1,AXIS3D *axis,POINT3D *p2)
{
//int i;
POINT3D temp;
//p2 might == p1

	temp.x = p1->x*axis->x[0] + p1->y*axis->y[0] + p1->z*axis->z[0];
	temp.y = p1->x*axis->x[1] + p1->y*axis->y[1] + p1->z*axis->z[1];
	temp.z = p1->x*axis->x[2] + p1->y*axis->y[2] + p1->z*axis->z[2];	
	memcpy(p2,&temp,sizeof(POINT3D));

}  //void PointMatrix3DMultiply(POINT3D *p1, AXIS3D *axis,POINT3D *p2, int numpoints)

//need points too because polygon refers to them by index #
void CalculatePolygonNormal(POLYGON *tpoly,POINT3D *tpoint) {
	POINT3D *normal,v1,v2;
	POINT3D p[3];
	int i;

	//normalized((v1-v2) X (v1-v3))

	for(i=0;i<3;i++) {
		memcpy(&p[i],&tpoint[tpoly->p[i]],sizeof(POINT3D));
	}

	normal=&tpoly->n;

	v1.x=(p[0].x-p[1].x);
	v1.y=(p[0].y-p[1].y);
	v1.z=(p[0].z-p[1].z);

	v2.x=(p[0].x-p[2].x);
	v2.y=(p[0].y-p[2].y);
	v2.z=(p[0].z-p[2].z);

	CalculateCrossProduct(&v1,&v2,normal);
	NormalizeVector(normal);
} //void CalculatePolygonNormal(POLYGON *tpoly,POINT3D *tpoint) {

void CalculateCrossProduct(POINT3D *v1,POINT3D *v2,POINT3D *v3) {
	v3->x=v1->y*v2->z - v1->z*v2->y;
	v3->y=v1->z*v2->x - v1->x*v2->z;
	v3->z=v1->x*v2->y - v1->y*v2->x;
} //void CalculateCrossProduct(POINT3D *v1,POINT3D *v2,POINT3D *v3) {

void NormalizeVector(POINT3D *v) {
	float sum;

	sum=sqrt((v->x*v->x)+(v->y*v->y)+(v->z*v->z));
	v->x/=sum;
	v->y/=sum;
	v->z/=sum;
} //void NormalizeVector(POINT3D *v) {


//todo: each scene (render window/image) must have its own zbuf,tbuf, etc.
//currently they use F3DStatus and so only 1 render window is possible
//if the windows/images are all the same size it would be ok since zbuf and tbuf would be the same size- they could be reused 
void RenderSceneFromCamera(u8 *bmp,int w,int h,CAMERA *lcamera,int upsidedown)
{
	u32 i,k;
	//long int lastTickCount = 0;
	//int currentFrame = 0;
	//int haveBackground = 0;
	int ClipState;
	POLYGON tpolygon;
	F3D_LINE line;
	int offset;
	int ExcludedModel;
	EXCLUDED *lexclude;

	//unsigned char byt;
	MODEL *lmodel;
	//MOTION *lmotion;
	//i32 gotonextinst;//,setoaxis;  //possibly make last 2 bit variables/flags
	//long double tratio,tratio2;
//	MOTIONLIST *lmotlist,*next;
	OBJLIST *lobjlist;
	//POINT3D tp,tt;//,ti;
	//char tstr[255];
	//#if TRANSPARENCY
	//float fir,fig,fib,curtransp;
	//float nontdepth;
	//float totaltransp,curdepth;
	//i32 count;//,sn,en,cn;
	//#endif //TRANSPARENCY
	BITMAPINFOHEADER *bmi;


#if Linux
	//Lock RenderLock mutex
	pthread_mutex_lock(&F3DStatus.RenderLock);
#endif //Linux

	offset=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
	bmi=(BITMAPINFOHEADER *)(bmp+sizeof(BITMAPFILEHEADER));


	if (bmi->biBitCount!=24 && bmi->biBitCount!=32) {
		FTMessageBox("Bitmap to render to needs to be either 24 or 32 bits per pixel",FTMB_OK,"Error",0);
		return;
	}
	//clear screen
	//memset(screen,0x0,WIDTH*HEIGHT*sizeof(PIXELZ));
	if (!(F3DStatus.renderflags&F3D_RENDER_NO_SCREEN_REFRESH)) {
		if (bmi->biBitCount==32) {
			if (F3DStatus.renderflags&F3D_RENDER_WHITE_BACKGROUND) {
				memset(bmp+offset,0xff,w*h*4);
			} else {
				memset(bmp+offset,0x0,w*h*4);
			}
		} //if (bmi->biBitCount==32) {

		if (bmi->biBitCount==24) {
			if (F3DStatus.renderflags&F3D_RENDER_WHITE_BACKGROUND) {
				memset(bmp+offset,0xff,w*h*3);
			} else {
				memset(bmp+offset,0x0,w*h*3);
			}
		} //if (bmi->biBitCount==24) {
	} //if (!(F3DStatus.renderflags&F3D_RENDER_REFRESH_SCREEN)) {

	//allocate and clear transparency buffer
	//float tbuf[MAX_RENDER_WIDTH][MAX_RENDER_HEIGHT];
	if (F3DStatus.renderflags&F3D_RENDER_USE_TRANSPARENCY) {
		//allocate transparency buffer memory if not allocated already
		if (F3DStatus.tbuf==0) {
			F3DStatus.tbuf=(TRANSPARENCY_BUFFER *)malloc(w*h*sizeof(TRANSPARENCY_BUFFER));
		} //if (F3DStatus.tbuf==0) {

		//there is a problem in that accessing tbuf[w][h] for smaller dimensions
		//because unless clearing MAX_RENDER_WIDTH*MAX_RENDER_HEIGHT not all of the zbuf will get cleared
		//zbuf and tbuf need to be handled as *(zbuf+offsetx+offsety*width*sizeof(float))
	//memset((u8 *)tbuf,0x0,WIDTH*HEIGHT*(sizeof(float)));  //tbuf is float (4 bytes), init to 0.0
		memset((u8 *)F3DStatus.tbuf,0x0,w*h*(sizeof(TRANSPARENCY_BUFFER)));  //tbuf is float (4 bytes), init to 0.0
	} //if (F3DStatus.renderflags&F3D_RENDER_USE_TRANSPARENCY) {

	//allocate z buffer memory if not allocated already
	if (F3DStatus.zbuf==0) {
		F3DStatus.zbuf=(float *)malloc(w*h*sizeof(float));
	} //if (F3DStatus.zbuf==0) {
	
	//clear z buffer	
	//float zbuf[MAX_RENDER_WIDTH][MAX_RENDER_HEIGHT];
	if (F3DStatus.renderflags&F3D_RENDER_USE_Z_BUFFER) {
		//there is a problem in that accessing zbuf[w][h] for smaller dimensions
		//because unless clearing MAX_RENDER_WIDTH*MAX_RENDER_HEIGHT not all of the zbuf will get cleared
		memset((u8 *)F3DStatus.zbuf,0x0,w*h*(sizeof(float)));  //zbuf is float (4 bytes), init to 0.0
	//memset((u8 *)zbuf,0xff,WIDTH*HEIGHT*(sizeof(float)));  //zbuf is float (4 bytes), init to -1.0
	} //if (F3DStatus.renderflags&F3D_RENDER_USE_Z_BUFFER) {

#if 0 
	for(j=0;j<h;j++) {
	  for(i=0;i<w;i++) {
			zbuf[i][j]=-1.0;
			} //i
	} //j
#endif

	//now go thru all objects and draw models
	//lmodel=imodel;
	//while(lmodel!=0) {
	//possibly just go through all models with imodel?
	lobjlist=F3DStatus.iobjlist;
	while(lobjlist!=0) {
		if (lobjlist->type=='m') {  //only process models
			lmodel=lobjlist->model;

			//from here on is all models
			if (!(lmodel->flags&MODEL_NORENDER)) {

				//go thru ExcludeModel list from camera for this camera
				//do not draw any models that are excluded to this camera
				lexclude=lcamera->exclude;
				ExcludedModel=0;
				while(lexclude!=0) {
					if (strcmp(lmodel->name,lexclude->ModelName)==0) {
							ExcludedModel=1;
							lexclude=0;
					} else {
						lexclude=lexclude->next;
					}
				} //while
		
				if (!ExcludedModel) {


					if (F3DStatus.renderflags&F3D_RENDER_PERSPECTIVE) {
						//Determine tangent of camera focal length and screen x and y
						//todo: only calculate when curent camera changes
						F3DStatus.tanFOV2x=(w-1)/(lcamera->focus*2);
						F3DStatus.tanFOV2y=(h-1)/(lcamera->focus*2);
					}  else {
						F3DStatus.WinMaxX=w-1.0;
						F3DStatus.WinMaxY=h-1.0;
					}

					//APPLY TRANSFORMS:
					if (F3DStatus.renderflags&F3D_RENDER_TRANSFORM_POINTS_BY_MODEL_AXES) {
						//TRANSFORM MODEL POINTS WITH MODEL AXES (AND LOCATION)
						//move points relative to all model axes
						//transforms lmodel->point and puts them into lmodel->apoint
						//and also transforms lmodel->normal (polygon normals) and puts into lmodel->anormal
						//also transforms polygon->n to polygon->an
						TransformModelByAxes(lmodel);
						//PointMatrix3DMultiplyAllAxes(lmodel); 

						//TRANSFORM POINTS WITH CAMERA AXIS AND LOCATION
						//(camera transform is also known as universe, world or viewer transform - rotate and transform all model points relative to camera)
						//apply camera transform to lmodel->apoint, and put into lmodel->cpoint which can be clipped
						PointsMatrix3DMultiplyToClipPoints(lmodel->apoint,lcamera->axis,lcamera->location,lmodel->cpoint,lmodel->numpoint);  
						//TRANSFORM NORMALS WITH CAMERA AXIS AND LOCATION - so LightPolygon will work correctly
						PointsMatrix3DMultiply(lmodel->anormal, &lcamera->axis, lmodel->anormal,lmodel->numnormal);						
					} else {
						//TRANSFORM POINTS WITH CAMERA AXIS AND LOCATION
						//(camera transform is also known as universe, world or viewer transform - rotate and transform all model points relative to camera)
						//apply camera transform to lmodel->apoint, and put into lmodel->cpoint which can be clipped
						PointsMatrix3DMultiplyToClipPoints(lmodel->point,lcamera->axis,lcamera->location,lmodel->cpoint,lmodel->numpoint);  
						//set any polygons as culled if cull flag is set here?
						//if (F3DStatus.renderflags&F3D_RENDER_CULL_BACKFACING_POLYGONS) {
						//	CullTransformedPolygons(lmodel);
						//}if (F3DStatus.renderflags&F3D_RENDER_CULL_BACKFACING_POLYGONS) {
					} //if (F3DStatus.renderflags&F3D_RENDER_TRANSFORM_POINTS_BY_MODEL_AXES) {

					//object clipping: See if entire model boundary is clipped

					//polygon clipping
					if (F3DStatus.renderflags&F3D_RENDER_POLYGON_CLIPPING) {

						//set clip flag for every point
						if (F3DStatus.renderflags&F3D_RENDER_PERSPECTIVE) {
							for(i=0;i<lmodel->numpoint;i++) {
								lmodel->cpoint[i].Flag=IsPointClippedPerspective(lmodel->cpoint[i]);
								}
						} else {
							for(i=0;i<lmodel->numpoint;i++) {
								lmodel->cpoint[i].Flag=IsPointClipped(lmodel->cpoint[i]);
								}
						}
					} //if (F3DStatus.renderflags&F3D_RENDER_POLYGON_CLIPPING) {

					//LINES ONLY
					//clip and draw all lines in model - an alternative is to view all lines as 2 (or more) point polygons whose faces are not rendered
					for(k=0;k<lmodel->numline;k++) {
						F3DStatus.LineFlag=0;
						//for each point in the line (currently only 2), determine their position after being clipped
						for(i=0;i<2;i++) {
						  F3DStatus.clippoint[i].x=lmodel->cpoint[lmodel->line[k].p[i]].x;
						  F3DStatus.clippoint[i].y=lmodel->cpoint[lmodel->line[k].p[i]].y;
						  F3DStatus.clippoint[i].z=lmodel->cpoint[lmodel->line[k].p[i]].z;
						  F3DStatus.clippoint[i].Flag=lmodel->cpoint[lmodel->line[k].p[i]].Flag;
						  }
						ClipState=IsPolygonOrLineClipped(F3DStatus.clippoint,2,&F3DStatus.LineFlag);
						if (ClipState!=TOTAL_CLIP) {
							//For those lines not totally clipped
							line=lmodel->line[k];

							if (ClipState==PARTIAL_CLIP) { //line is clipped, redefine vertices
								ClipLine();
							}

							//transform polygon to screen coordinates
							
							//transform perspective - take care of reverse y component here because in models y+ is up
							if (F3DStatus.renderflags&F3D_RENDER_PERSPECTIVE) {			
								for(i=0;i<2;i++) {								
									F3DStatus.clippoint[i].x*=(lcamera->focus/F3DStatus.clippoint[i].z);
									F3DStatus.clippoint[i].y*=-(lcamera->focus/F3DStatus.clippoint[i].z); 
								}
							} else {
								F3DStatus.clippoint[i].y=-F3DStatus.clippoint[i].y; 
							}

							//transform translationally
							for(i=0;i<2;i++) {
								F3DStatus.clippoint[i].x+=w/2.0;
								F3DStatus.clippoint[i].y+=h/2.0;								
								}

							//draw clipped, (lighted), transformed, 2d line
							DrawLine(bmp,w,h,F3DStatus.clippoint,line,lmodel->texture,upsidedown);

					}  //if (ClipState!=TOTAL_CLIP) {

				}  //for(k=0;k<lmodel->numline;k++) {




				//clip and draw all polygons in model
				for(k=0;k<lmodel->numpolygon;k++) {
					F3DStatus.PolyFlag=0;
					//copy all transformed camera points into an array of global clippoints
					//todo: figure out way to avoid this when CLIPPING flag is not set
					for(i=0;i<lmodel->polygon[k].numpoint;i++)
						{
						F3DStatus.clippoint[i].x=lmodel->cpoint[lmodel->polygon[k].p[i]].x;
						F3DStatus.clippoint[i].y=lmodel->cpoint[lmodel->polygon[k].p[i]].y;
						F3DStatus.clippoint[i].z=lmodel->cpoint[lmodel->polygon[k].p[i]].z;
						F3DStatus.clippoint[i].u=lmodel->polygon[k].u[i];
						F3DStatus.clippoint[i].v=lmodel->polygon[k].v[i];
						F3DStatus.clippoint[i].color=lmodel->polygon[k].color[i];
						F3DStatus.clippoint[i].Flag=lmodel->cpoint[lmodel->polygon[k].p[i]].Flag;
						}
					
					if (F3DStatus.renderflags&F3D_RENDER_POLYGON_CLIPPING) { //clipping is enabled

						ClipState=IsPolygonOrLineClipped(F3DStatus.clippoint,lmodel->polygon[k].numpoint,&F3DStatus.PolyFlag);
					} else {
						ClipState=NO_CLIP;
					} //if (F3DStatus.renderflags&F3D_RENDER_POLYGON_CLIPPING) { //clipping is enabled

					if (ClipState!=TOTAL_CLIP) {
						//For those polygons not totally clipped
						//exclude/cull backfacing polygons
						//Culling needs to add perspective because z plane rectangles are culled otherwise - but are visible because of perspective
						if (!((F3DStatus.renderflags&F3D_RENDER_CULL_BACKFACING_POLYGONS) && (F3DStatus.clippoint[0].x*(F3DStatus.clippoint[2].y-F3DStatus.clippoint[1].y) +   //only pass counter-clockwise
							F3DStatus.clippoint[1].x*(F3DStatus.clippoint[0].y-F3DStatus.clippoint[2].y) + 
    						F3DStatus.clippoint[2].x*(F3DStatus.clippoint[1].y-F3DStatus.clippoint[0].y))<0.0))    {//<0.0 clock, >0counterclock

							//copy polygon into a local variable so we can change number of points and color of points
							//polygon=lmodel->polygon[k];
							memcpy(&tpolygon,&lmodel->polygon[k],sizeof(POLYGON));

							//apply lighting - only if rendering polygon faces and lighting
							if ((F3DStatus.renderflags&F3D_RENDER_POLYGON_FACES) && (F3DStatus.renderflags&F3D_RENDER_USE_LIGHTING)) {
								if (!(lmodel->flags&MODEL_NOLIGHTING)) {  //don't light models with this bit set
									//light polygon here, vertex color will be interpolated
									//otherwise a new surface normal would need to be calculated for each clipped vertex
									//Note: Lighting the polygon must use the the untransformed ->point (or ->apoint when model axes are applied)
									//otherwise the lighting changes depending on the camera.
									//In addition, lighting lights the unclipped polygon vertices, 
									//and the colors are interpolated for clipped polygons
									LightPolygon(lmodel,&tpolygon);
									//update clippoint lighting 
									//todo: store the lit polygon colors in lcolor[4] so they will be stored if we don't need to relight, but do need to redraw the scene
									for(i=0;i<tpolygon.numpoint;i++) {
										F3DStatus.clippoint[i].color=tpolygon.color[i];
									}
								} //if (!(lmodel->flags&MODEL_NOLIGHTING)) {  //don't light models with this bit set
							} //if ((F3DStatus.renderflags&F3D_RENDER_POLYGON_FACES) && (F3DStatus.renderflags&F3D_RENDER_USE_LIGHTING)) {

							if (ClipState==PARTIAL_CLIP) { //polygon is clipped, redefine vertices
								//tpolygon.numpoint=ClipPolygon(lmodel->polygon[k].numpoint);
								tpolygon.numpoint=ClipPolygon(tpolygon.numpoint);
							}
							
							//transform polygon for perspective
							//take care of reverse y component here because in models y+ is up
							if (F3DStatus.renderflags&F3D_RENDER_PERSPECTIVE) {			
								for(i=0;i<tpolygon.numpoint;i++) {
									F3DStatus.clippoint[i].x*=(lcamera->focus/F3DStatus.clippoint[i].z);
									F3DStatus.clippoint[i].y*=-(lcamera->focus/F3DStatus.clippoint[i].z); 
								}
							} else {
								F3DStatus.clippoint[i].y=-F3DStatus.clippoint[i].y; 
							}

							//transform polygon to screen coordinates
							for(i=0;i<tpolygon.numpoint;i++) {
								F3DStatus.clippoint[i].x+=w/2;
								F3DStatus.clippoint[i].y+=h/2;
								}

							//draw final 2d polygon							
							DrawPolygon(bmp,w,h,F3DStatus.clippoint,tpolygon,lmodel->texture,upsidedown);

							} //if ((F3DStatus.renderflags&F3D_RENDER_CULL_BACKFACING_POLYGONS) || (F3DStatus.clippoint[0].x*(F3DStatus.clippoint[2].y-F3DStatus.clippoint[1].y) +   //only pass counter-clockwise
						}  //if (ClipState!=TOTAL_CLIP) {
				}  //for(k=0;k<lmodel->numpolygon;k++) {


	#if 0 
			//advance video textures one frame if any textures in this model are videos
			for(k=0;k<lmodel->numtexture;k++) {
				//or use .avi file extension
				if (lmodel->tbits[k]&MODEL_VIDEOTEXTURE) {
		
					//are there more frames? 
					if (lmodel->curframe[k]<lmodel->frames[k]) {
					//yes=read in the next frame
						ReadVideoFrame(lmodel,k);
						if (lmodel->curframe[k]==lmodel->frames[k]-1) { //loop or close video file
							//close file pointer if done
						  fclose(lmodel->tfptr[k]);
						} //last frame
						lmodel->curframe[k]++;//advance frame
					}  //lmodel->curframe[h]<lmodel->frames[h]  
				}  //MODEL_VIDEOTEXTURE

				//check for camera texture

	#if 0 
				//this causes an infinite loop
				if (*(lmodel->texturename+k*TEXTURENAMESIZE)==64) { //"@") 
					//LoadTexture(lmodel,k,lmodel->texturename+k*TEXTURENAMESIZE);
							strcpy(tstr,(lmodel->texturename+k*TEXTURENAMESIZE+1));
							lcamera=GetCamera(tstr);
							//strcpy(lmodel->texturename+TEXTURENAMESIZE*ti,name);
							//bitmap returned can be either upside-down or right-side-up
							//we need to have it upside down because it is a texture and bitmaps are upside down
							RenderSceneFromCamera((u8 *)lmodel->texture[k],WIDTH,HEIGHT,lcamera,1); //render scene to screen bitmap from the current camera's perspective

				} //if (lmodel->texture[k][0]==64) { //"@") {
	#endif

			} //k
	#endif
				} //			if (!ExcludedModel) {

			}		//if (!(lmodel->flags&MODEL_NORENDER)) {

		}  //if (lobjlist->type=='m') {  //only process models
		
	lobjlist=lobjlist->next;  //go to next object
	}  //	while(lobjlist!=0) //(for each object)

#if Linux
	//unlock RenderLock mutex
	pthread_mutex_unlock(&F3DStatus.RenderLock);
#endif //Linux


} //void RenderSceneFromCamera(u8 *bmp,CAMERA *lcamera)


//Light the polygon vertices, using either the original points, or the points transformed by the model axes
void LightPolygon(MODEL *lmodel,POLYGON *tpolygon) {
	unsigned char tr,tg,tb,lr,lg,lb;
	POINT3D VL;
	float Angle,Distance;
	int i;
	OBJLIST *tobjlist;
	LIGHT *llight;
	POINT3D *tp[4]; //points
	POINT3D *tn[4]; //normals
	//POINT3D *vl[4]; //vector between point on polygon and point of light

	//use the precomputed and axis and camera translated normal of the polygon for flat shading
	//and the precomputed and axis and camera translated normal of each vertex for Gouraud shading

	//create a local variable to store either the original polygon points or the points transformed by the model axes
	if (lmodel->normal==0) {
		tpolygon->flags|=F3D_POLYGON_USE_POLYGON_NORMAL; //if no flag set anormal[]==0 and mem fault
	}

	if (F3DStatus.renderflags&F3D_RENDER_TRANSFORM_POINTS_BY_MODEL_AXES) {
		//use the polygon points transformed by the model axes
		for(i=0;i<tpolygon->numpoint;i++) {
			tp[i]=&lmodel->apoint[tpolygon->p[i]];
			//tp[i] = &lmodel->cpoint[tpolygon->p[i]];
			if ((tpolygon->flags&F3D_POLYGON_USE_POLYGON_NORMAL) || (F3DStatus.renderflags&F3D_RENDER_FLAT_SHADING)) {
				tn[i]=&tpolygon->an;
			} else {
				tn[i]=&lmodel->anormal[tpolygon->p[i]];
			}
		}
	} else {
		//use the original polygon points
		for(i=0;i<tpolygon->numpoint;i++) {
			tp[i]=&lmodel->point[tpolygon->p[i]];
			if ((tpolygon->flags&F3D_POLYGON_USE_POLYGON_NORMAL) || (F3DStatus.renderflags&F3D_RENDER_FLAT_SHADING)) {
				tn[i]=&tpolygon->n;
			} else {
				tn[i]=&lmodel->normal[tpolygon->p[i]];
			}
		}
	} //if (F3DStatus.renderflags&F3D_RENDER_TRANSFORM_POINTS_BY_MODEL_AXES) {
/*
	//find normal of polygon plane
	V0.x=F3DStatus.clippoint[1].x-F3DStatus.clippoint[0].x; V0.y=F3DStatus.clippoint[1].y-F3DStatus.clippoint[0].y;  V0.z=F3DStatus.clippoint[1].z-F3DStatus.clippoint[0].z;
	V1.x=F3DStatus.clippoint[2].x-F3DStatus.clippoint[0].x; V1.y=F3DStatus.clippoint[2].y-F3DStatus.clippoint[0].y;  V1.z=F3DStatus.clippoint[2].z-F3DStatus.clippoint[0].z;

	N.x=V0.y*V1.z-V1.y*V0.z;
	N.y=V0.x*V1.z-V1.x*V0.z;
	N.z=V0.x*V1.y-V1.x*V0.y;

	//make normal from range 0 to 1
	Denominator=(float)sqrt(N.x*N.x+N.y*N.y+N.z*N.z);
	N.x/=Denominator;
	N.y/=Denominator;
	N.z/=Denominator;
*/

	//if flat shading, polygon vertices all have the same color based 
	//on the dot product of the polygon normal and the vector between 
	//the polygon and the light
	//if Gouraud shading, each vertices has a color based on the dot 
	//product of the polygon normal and the vector between the vertex 
	//normal and the light-note that the vertex normal may be different 
	//from the polygon normal
	//for each point in polygon 
	for(i=0;i<tpolygon->numpoint;i++) {
		//ignore existing polygon color
		tr=0; tg=0;  tb=0; 

		//go thru object list will take more time
		//but then only 1 scene list for lights
		//todo: separate ilight list for scene
		tobjlist=F3DStatus.iobjlist;
		while(tobjlist!=0) {
			if (tobjlist->type=='l') {
				llight=tobjlist->light;
				//get color of light
				lr=(llight->color&0x00ff0000)>>0x10;
				lg=(llight->color&0x0000ff00)>>0x8;
				lb=llight->color&0x000000ff;

				//if (light->type==0)
				if (llight->deg==0) {  //ambient light
					//add polygon color to light color
					if ((tr+lr)>0xff) {
						tr=0xff;
					} else {
						tr+=lr;
					}
					if ((tg+lg)>0xff) {
						tg=0xff;
					} else {
						tg+=lg;
					}
					if ((tb+lb)>0xff) {
						tb=0xff;
					} else {
						tb+=lb;
					}
				}  //deg==0


				if (llight->deg>0) { //physical light
					//calculate vector from light to vertex

					//clockwise or counter-clockwise makes a difference here
					//note: only the original points (->point), or points transformed by model axes (->apoint) should be lit
					//the points transformed by the camera should not be lit or else the lighting changes depending on 
					//where the viewer (camera) is
					//VL.x=llight->location.x-F3DStatus.clippoint[j].x;  VL.y=llight->location.y-F3DStatus.clippoint[j].y;  VL.z=llight->location.z-F3DStatus.clippoint[j].z;

					//get the vector between the point of the light and the point of the polygon
					//GetVector(&llight->location,tp[i],&vl);
					VL.x=llight->location.x-tp[i]->x;  VL.y=llight->location.y-tp[i]->y;  VL.z=llight->location.z-tp[i]->z;

					//and normalized it (set it to unit value ex: .886i+.5j+0k)
					//NormalizeVector(&vl);
					Distance=(float)sqrt((VL.x*VL.x)+(VL.y*VL.y)+(VL.z*VL.z));
					VL.x/=Distance;
					VL.y/=Distance;
					VL.z/=Distance;

					//calculate the angle between the polygon-light vector and the precomputed polygon point surface normal
					Angle=(VL.x*tn[i]->x+VL.y*tn[i]->y+VL.z*tn[i]->z);

					//from the angle calculate the intensity of the light 
					//and mulitiply that by the light color
					//(llight->intensity is currently not used and probably should be derived from light color? and distance of light to polygon)
					//todo: also divide by distance
					if (Angle>0.0) {  //cosine of angle 180 degrees
						lr=(unsigned char)((float)lr*Angle);
						if ((tr+lr)>0xff) {
							tr=0xff;
						} else {
							tr+=lr;  
						}
						lg=(unsigned char)((float)lg*Angle);
						if ((tg+lg)>0xff) {
							tg=0xff;
						} else {
							tg+=lg;  
						}
						lb=(unsigned char)((float)lb*Angle);
						if ((tb+lb)>0xff) {
							tb=0xff;
						} else {
							tb+=lb;  
						}
					}  //Angle<0
				}  //end deg>0

			} //end if objlist-?type=='l'
			tobjlist=tobjlist->next;		 
			//llight=llight->next;		 
		}  //while(tobjlist!=0) {
		lr=tr;  lg=tg;  lb=tb;

		//set the color of the vertex
		tpolygon->color[i]=(unsigned int) 0xff<<0x18 | 
			(unsigned int)(lr)<<0x10 | 
			(unsigned int)(lg)<<0x8 | 
			(unsigned int)(lb);

	}  //end i (for each point in polygon)
} //void LightPolygon(MODEL *lmodel,POLYGON *tpolygon) {

//void DrawPolygon(CLIPPOINT3D *newpnt, POLYGON polygon, unsigned int *texture)
//void DrawPolygon(u8 *bmp,int w, int h, CLIPPOINT3D *newpnt, POLYGON polygon, unsigned int *texture,int upsidedown)
void DrawPolygon(u8 *bmp,int w, int h, CLIPPOINT3D *newpnt, POLYGON polygon, TEXTUREMAP *texture,int upsidedown)
{
	i32 j,k,l,offset;
	u32 i,low,high;
	float ex,ey,ez,mx,mz,mu,mv;
	float sx,sy,sz,rx,ry,du,dv;
	float su,eu,sv,ev;
	float fr,fg,fb;
	u32 p0,p1;
	u32 cp;
	u32 sc,ec;  //start color end color
	u32 ir,ig,ib;
	float mr,mg,mb,r,g,b,dx,dy,dxdy,dydx;
	BITMAPFILEHEADER *bmf,*tbmf;
	BITMAPINFOHEADER *bmi,*tbmi;
	i32 bwidth,bheight,bdepth,boffset;
	u32 curcolor;
	float tx,ty;
	u8 *pixeloffset,*bmpdata;
	u8 *tdata;
	u8 *taddress;
	float *zbuf; //quick pointer to zbuffer
	TRANSPARENCY_BUFFER *tbuf; //quick pointer to transparency buffer	
	//i32 zoffset,toffset; //offset for zbuf and tbuf pointers


//#if TEXTUREMAP
	i32 twidth,theight,tdepth;
//picwidth,picdepth
//#endif //TEXTUREMAP
	//float mc,ic;
//#if TRANSPARENCY 
	float fir,fig,fib;  //for partial transparency, floating point values of color components
	float totaltransp;
	i32 count,curtransp,zorder,count2;//,transpskip,count3,zcount;
	float curtranspval;//,curdepth,nontdepth,thisz;
//#endif //TRANSPARENCY
	POLYLIST *pbuf;	
	int incdec;

//todo: pass width,height, depth and data buffer
	bmf=(BITMAPFILEHEADER *)bmp;
	bmi=(BITMAPINFOHEADER *)((u8 *)bmp+sizeof(BITMAPFILEHEADER));

	bwidth=bmi->biWidth;
	bheight=bmi->biHeight;
	bdepth=bmi->biBitCount;
	boffset=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
	bmpdata=bmp+boffset; //pointer to bitmap data

	if ((F3DStatus.renderflags&F3D_RENDER_USE_TEXTUREMAPS) && texture!=0 && texture->bmp!=0) {
		//Get Texture Map dimensions and depth
		tbmf=(BITMAPFILEHEADER *)texture->bmp;//(BITMAPFILEHEADER *)texture->bmp;
		tbmi=texture->bmpinfo;//(BITMAPINFOHEADER *)((u8 *)texture->bmp+sizeof(BITMAPFILEHEADER));
		tdata=texture->bmpdata;
		twidth=tbmi->biWidth;
		theight=tbmi->biHeight;
		tdepth=tbmi->biBitCount/8;
	}

	if (F3DStatus.renderflags&F3D_RENDER_POINTS_ONLY) {
		if (bmi->biBitCount==32) {
			for(i=0;i<polygon.numpoint;i++) {  
				pixeloffset=(u8 *)(bmpdata+(unsigned int)newpnt[i].y*w*4+(unsigned int)newpnt[i].x*4);
				//memset(pixeloffset,0xff,3);
				memcpy(pixeloffset,&polygon.color[i],3);
			} 
		} else {  //presumes 24-bit
			for(i=0;i<polygon.numpoint;i++) {  
				pixeloffset=(u8 *)(bmpdata+(unsigned int)newpnt[i].y*w*3+(unsigned int)newpnt[i].x*3);
				//memset(pixeloffset,0xff,3);
				memcpy(pixeloffset,&polygon.color[i],3);
			} 
		} //if (bmi->biBitCount==32) {
	} //if (F3DStatus.renderflags&F3D_RENDER_POINTS_ONLY) {

	if (F3DStatus.renderflags&F3D_RENDER_LINES_ONLY) {
		for(i=0;i<polygon.numpoint;i++) {  

			//determine the dx and dy
			if (i==polygon.numpoint-1) {  //last point
				p0=i;  p1=0;
			} else {
				p0=i;
				p1=i+1;
			} //if (i==polygon.numpoint-1) {  //last point

			dx=newpnt[p1].x-newpnt[p0].x; //note that point order is reversed
			dy=newpnt[p1].y-newpnt[p0].y;
			if (fabs(dx)>fabs(dy)) {  //change in x is greater than change in y, so increment along x
				dydx=dy/fabs(dx); //dx will never be 0 because fabs(dx)>fabs(dy) 
				ty=newpnt[p0].y;
				if (dx>0) {  //dx>0
					incdec=1;
				} else {
					incdec=-1;
				}
				if (bmi->biBitCount==32) {
					for(j=(u32)newpnt[p0].x;j!=(u32)newpnt[p1].x;j+=incdec) {
						pixeloffset=(u8 *)(bmpdata+(unsigned int)ty*w*4+4*j);
						//memset(pixeloffset,0xff,3);
						memcpy(pixeloffset,&polygon.color[i],3);
						ty+=dydx;
					}
				} else {
					for(j=(u32)newpnt[p0].x;j!=(u32)newpnt[p1].x;j+=incdec) {
						pixeloffset=(u8 *)(bmpdata+(unsigned int)ty*w*3+3*j);
						//memset(pixeloffset,0xff,3);
						memcpy(pixeloffset,&polygon.color[i],3);
						ty+=dydx;
					}					
				} //if (bmi->biBitCount==32) {
			} else { //change in y is greater than change in x, so increment along y
				if (dy==0) { //can only happen if two points in a line are identical- just to avoid a crash
					dxdy=0.0; 
				} else {
					dxdy=dx/fabs(dy);  //dy will never = 0 because there is no way dx==dy==0 except for two identical points
				}
				tx=newpnt[p0].x;
				if (dy>=0) {
					incdec=1;
				} else {
					incdec=-1;
				}
				if (bmi->biBitCount==32) {
					for(j=(u32)newpnt[p0].y;j!=(u32)newpnt[p1].y;j+=incdec) {
						pixeloffset=(u8 *)(bmpdata+j*w*4+(unsigned int)tx*4);
						//memset(pixeloffset,0xff,3);
						memcpy(pixeloffset,&polygon.color[i],3);
						tx+=dxdy;
					}
				} else {
					for(j=(u32)newpnt[p0].y;j!=(u32)newpnt[p1].y;j+=incdec) {
						pixeloffset=(u8 *)(bmpdata+j*w*3+(unsigned int)tx*3);
						//memset(pixeloffset,0xff,3);
						memcpy(pixeloffset,&polygon.color[i],3);
						tx+=dxdy;
					}
				} //if (bmi->biBitCount==32) {
			} //if (fabs(dx)>fabs(dy)) {  //change in x is greater than change in y, so increment along x

		} //for(i=0;i<polygon.numpoint;i++) {  
	} //if ((F3DStatus.renderflags&F3D_RENDER_LINES_ONLY)) {


	if (F3DStatus.renderflags&F3D_RENDER_POLYGON_FACES) {
		low=bheight/2;//241;//500;  //>480x20
		high=0;//-1;  //<0

		//only include polygons that have at least one point with positive z
		//should already be clipped to Z=0 plane above
//		if (newpnt[0].z>0.0 || newpnt[1].z>0.0 || newpnt[2].z>0.0 || (polygon.numpoint>3 && newpnt[3].z>0.0)) {
		for(i=0;i<polygon.numpoint;i++) { //for each point in the polygon get the highest and lowest y
			//check for highest and lowest
			if(low>newpnt[i].y) {
				low=(u32)newpnt[i].y; 
			}
			if(high<newpnt[i].y) {
				high=(u32)newpnt[i].y; 
			}
		} //for(i=0;i<polygon.numpoint;i++) { //for each point in the polygon get the highest and lowest y
	
		pbuf=(POLYLIST *)&F3DStatus.pbuf;
		memset(pbuf,0,sizeof(POLYLIST)*MAX_RENDER_HEIGHT);  //store each x on each horizontal line

		for(i=0;i<polygon.numpoint;i++)  { //for each point in the polygon
			//add each line (edge) in the polygon to the vertical line buffer (pbuf)
			//determine the top and bottom point for the line
			if (i==polygon.numpoint-1) {
				p0=i;  p1=0;
			} else {
				p0=i;
				p1=i+1;
			}
	
		if (newpnt[p0].y<=newpnt[p1].y) {
			sx=newpnt[p0].x;
			sy=newpnt[p0].y;  //start of line
			sz=newpnt[p0].z;
			su=newpnt[p0].u;  
			sv=newpnt[p0].v;
			//sc=polygon.color[p0];
			//osc=polygon.color[p0];
			sc=newpnt[p0].color;
			//osc=newpnt[p0].color;
			ex=newpnt[p1].x;  //end of line
			ey=newpnt[p1].y;
			ez=newpnt[p1].z;
			eu=newpnt[p1].u;  
			ev=newpnt[p1].v;
			//ec=polygon.color[p1];
			ec=newpnt[p1].color;
		} else {
			sx=newpnt[p1].x;
			sy=newpnt[p1].y;  //start of line
			sz=newpnt[p1].z;
			su=newpnt[p1].u;  
			sv=newpnt[p1].v;
			//sc=polygon.color[p1];
			//osc=polygon.color[p1];
			sc=newpnt[p1].color;
			//osc=newpnt[p1].color;
			ex=newpnt[p0].x;    //end of line
			ey=newpnt[p0].y;
			ez=newpnt[p0].z;
			eu=newpnt[p0].u;  
			ev=newpnt[p0].v;
			//ec=polygon.color[p0];
			ec=newpnt[p0].color;
		} //		if (newpnt[p0].y<=newpnt[p1].y) {

		if (ey!=sy) {  //skip every horizontal line  
			//calculate the slop of each line
			dy=(ey-sy);  
			mx=(ex-sx)/dy; 
			mz=(ez-sz)/dy; 
			mu=(eu-su)/dy;
			mv=(ev-sv)/dy;

			//change in r g b

			mr=((float)((ec&0x00ff0000)>>0x10) - (float)((sc&0x00ff0000)>>0x10))/dy;
			mg=((float)((ec&0x0000ff00)>>0x8) - (float)((sc&0x0000ff00)>>0x8))/dy;
			mb=((float)(ec&0x000000ff) - (float)(sc&0x000000ff))/dy;

			r=(float)((sc&0x00ff0000)>>0x10);
			g=(float)((sc&0x0000ff00)>>0x8);
			b=(float)(sc&0x000000ff);

			
			//for each line from top to bottom of polygon
	//		for(j=(i32)sy+1;j<=(i32)ey;j++) 
			for(j=(i32)sy;j<(i32)ey;j++) {
				if (pbuf[j].numx>0) { // at least one point from previous polygons is on this screen line
					//for all points in pbuf 
					if (sz>0) {//point is in front of viewer
						//add x to list in buffer before any other larger x
						for(k=0;k<pbuf[j].numx;k++) { //
							if (sx<pbuf[j].x[k]) {
								//move back other x values
								for(l=pbuf[j].numx;l>=k;l--) {
									pbuf[j].x[l]=pbuf[j].x[l-1];
									pbuf[j].z[l]=pbuf[j].z[l-1];
									pbuf[j].u[l]=pbuf[j].u[l-1];
									pbuf[j].v[l]=pbuf[j].v[l-1];
									pbuf[j].c[l]=pbuf[j].c[l-1];
								}
								pbuf[j].x[k]=(i32)sx; 
								pbuf[j].z[k]=sz;
								pbuf[j].u[k]=su;
								pbuf[j].v[k]=sv;
								pbuf[j].c[k]=sc;
								pbuf[j].numx++;
								break;
							} else { //if (sx<pbuf[j].x[k]) {
								if (k==pbuf[j].numx-1)  { //last one
									pbuf[j].x[k+1]=(i32)sx; 
									pbuf[j].z[k+1]=sz;
									pbuf[j].u[k+1]=su;
									pbuf[j].v[k+1]=sv;
									pbuf[j].c[k+1]=sc;
									pbuf[j].numx++;
									break;
								}
							} //if (sx<pbuf[j].x[k]) {
						}  //end for k
					}//end sz clip
				} else { //1st point on screen horizontal line (scanline) 
					if(sz>0) {
						pbuf[j].x[0]=(i32)sx; 
						pbuf[j].z[0]=sz;
						pbuf[j].u[0]=su;
						pbuf[j].v[0]=sv;
						pbuf[j].c[0]=sc;
						pbuf[j].numx++;
					}  //if(sz>0)  end z clip
				} //if (pbuf[j].numx>0) { // at least one point from previous polygons is on this screen line
				sx+=mx;
				sz+=mz;
				su+=mu;
				sv+=mv;
				r+=mr; //this is the first linear interpolation of the between-vertices pixel's color (over change in y)
				g+=mg;
				b+=mb;

				sc=(unsigned int)r<<0x10 | (unsigned int)g<<0x8 | (unsigned int) b;
			}  //for(j=(i32)sy;j<(i32)ey;j++) {
		}//if (ey!=sy) {  //horizontal line  

	}  //for(i=0;i<polygon.numpoint;i++)  { //for each point (and line) in the polygon


	//actually draw polygon (draw and shade inside between point pairs)
	//for each scan line
	//fill in between x and next x
	for(i=low;i<high;i++) { //for each scan line
		for(j=0;j<pbuf[i].numx;j=j+2) { //for each polygon edge (always in pairs, a start and end, left side and right side)

			//for all x points on scanline
			sz=pbuf[i].z[j];  
			if (pbuf[i].x[j+1]!=pbuf[i].x[j]) { //same point
				//calculate change in x
				dx=(float)(pbuf[i].x[j+1]-pbuf[i].x[j]); 
				//calculate change in z over change in x
				//mz=(float)(pbuf[i].z[j+1]-pbuf[i].z[j])/mx; //note mx can=0 
				mz=(float)(pbuf[i].z[j+1]-pbuf[i].z[j])/dx; 
				//calculate change in texture coords over change in x
				du=(pbuf[i].u[j+1]-pbuf[i].u[j])/dx;
				dv=(pbuf[i].v[j+1]-pbuf[i].v[j])/dx;

				if (F3DStatus.renderflags&F3D_RENDER_USE_LIGHTING) {
					//change in r g b color between each point
					mr=((float)((pbuf[i].c[j+1]>>0x10)&0x000000ff) - (float)((pbuf[i].c[j]>>0x10)&0x000000ff))/dx;
					mg=((float)((pbuf[i].c[j+1]&0x0000ff00)>>0x8) - (float)((pbuf[i].c[j]&0x0000ff00)>>0x8))/dx;
					mb=((float)(pbuf[i].c[j+1]&0x000000ff) - (float)(pbuf[i].c[j]&0x000000ff))/dx;
				} //if ((F3DStatus.renderflags&F3D_RENDER_USE_LIGHTING) {

				//for each pixel in between two polygon scan line intersections
				for(k=pbuf[i].x[j];k<pbuf[i].x[j+1];k++) {
					//check new z points with any in bmp

					//todo: change this to a direct pointer to the pixel (like the zbuf and tbuf)
					//presumes 32-bit final bitmap
					if (upsidedown) {
						offset=(bheight-i-1)*w*4+k*4+boffset;
						//offset=i*w*4+k*4+boffset;
					} else {
						offset=i*w*4+k*4+boffset;
					}

					//calculate the pixel offset for the z buffer
					if (F3DStatus.renderflags&F3D_RENDER_USE_Z_BUFFER) {
						//zbuf=F3DStatus.zbuf+i*w*(sizeof(float))+k*(sizeof(float));
						zbuf=F3DStatus.zbuf+i*w+k;
					}

					//calculate the pixel offset for the transparency buffer
					if (F3DStatus.renderflags&F3D_RENDER_USE_TRANSPARENCY) {
						//tbuf=F3DStatus.tbuf+i*w*(sizeof(TRANSPARENCY_BUFFER))+k*(sizeof(TRANSPARENCY_BUFFER));
						tbuf=F3DStatus.tbuf+i*w+k;
					}

					//if (!(F3DStatus.renderflags&F3D_RENDER_USE_Z_BUFFER) || (F3DStatus.zbuf[k][i]>sz || F3DStatus.zbuf[k][i]==0.0)) ) {  //not using z-buffer or pixel is above z value for this pixel
					if (!(F3DStatus.renderflags&F3D_RENDER_USE_Z_BUFFER) || *zbuf>sz || *zbuf==0.0) {  //not using z-buffer or pixel is above z value for this pixel, or there is nothing on the pixel yet

			  		//z>0 and point has a smaller z than bmp pixel or pixel is unlit
						if ((F3DStatus.renderflags&F3D_RENDER_USE_TEXTUREMAPS) && texture!=0 && texture->bmp!=0) {
							//scaled image transfered to polygon
							rx=(float)(twidth-1)*(pbuf[i].u[j]);//+du); //without -1 there is a white spot on texture in last point
							ry=(float)(theight-1)*(pbuf[i].v[j]);//+dv);
							//presumes 32-bit final bitmap
							//cp=*((unsigned int *)tdata+(unsigned int)rx + (unsigned int)(twidth*((unsigned int)((theight-1)-(int)ry))));
							//handles both 24 and 32 bit texture bmps:
							//taddress=tdata+((unsigned int)rx + (unsigned int)(twidth*((unsigned int)((theight-1)-(int)ry))))*tdepth;
							//cp=((*(taddress+(tdepth==4))<<16)|(*(taddress+1+(tdepth==4))<<8)|*(taddress+2+(tdepth==4)));
							if (tdepth==4) {
								taddress=(unsigned char *)((unsigned int *)tdata+((unsigned int)rx + (unsigned int)(twidth*((unsigned int)((theight-1)-(int)ry)))));
								cp=((*(taddress+1)<<16)|(*(taddress+2)<<8)|*(taddress+3));
							} else {
							//	//24-bit bitmap
								taddress=tdata+((unsigned int)rx + (unsigned int)(twidth*((unsigned int)((theight-1)-(int)ry))))*tdepth;
								cp=((*(taddress)<<16)|(*(taddress+1)<<8)|*(taddress+2));
							}
							//handles 32-bit bitmap textures only:
							//taddress=((unsigned int *)tdata+(unsigned int)rx + (unsigned int)(twidth*((unsigned int)((theight-1)-(int)ry))));
							//if retrieved as an int is little endian
							//cp=(unsigned int)((*(taddress+1)<<16)|(*(taddress+2)<<8)|*(taddress+3));


						} //if (F3DStatus.renderflags&F3D_RENDER_USE_TEXTUREMAP) 
					 
						if (F3DStatus.renderflags&F3D_RENDER_USE_LIGHTING) {
						//todo: add Gouraud shading instead of average

							if ((F3DStatus.renderflags&F3D_RENDER_USE_TEXTUREMAPS) && texture!=0 && texture->bmp!=0) {
								//add the texture map color to the color of the point inside the polygon on the scanline
								if (!((F3DStatus.renderflags&F3D_RENDER_USE_TRANSPARENT_COLOR_KEY) && cp==F3DStatus.transparent_key_color)) { //not the transparent color
									//scale down texture map color with polygon color (which is determined by lighting)			
									//what c[j] is divided by is how intense the light is (other than the calculation of how intense the light is from the distance)
									fb=((float)((pbuf[i].c[j]>>0x10)&0x000000ff)*(float)((cp>>0x10)&0x000000ff))/200.0;
									fg=((float)((pbuf[i].c[j]>>8)&0x000000ff)*(float)((cp>>8)&0x000000ff))/200.0;
									fr=((float)(pbuf[i].c[j]&0x000000ff)*(float)(cp&0x000000ff))/200.0;

									//any color >1.0 set to 1.0
									if (fr>255.0) {
										ir=0xff;
									} else {
										ir=(u32)fr;
									}
									if (fg>255.0) {
										ig=0xff;
									} else {
										ig=(u32)fg;
									}
									if (fb>255.0) {
										ib=0xff;
									} else {
										ib=(u32)fb;
									}	
								} //if (!(F3DStatus.renderflags&F3D_RENDER_USE_TRANSPARENT_COLOR_KEY) && cp!=F3DStatus.transparent_key_color) { //not the transparent color
							} else {
								//USING LIGHTING BUT NOT TEXTURE MAPS
								cp=pbuf[i].c[j];
								ir=(float)((cp>>0x10)&0x000000ff);
								ig=(float)((cp>>8)&0x000000ff);
								ib=(float)(cp&0x000000ff);
							} //if (F3DStatus.renderflags&F3D_RENDER_USE_TEXTUREMAPS && texture!=0 && texture.bmp!=0 {
						} else { //if (F3DStatus.renderflags&F3D_RENDER_USE_LIGHTING) 
							//NOT USING LIGHTING BUT USING TEXTURE MAPS
							if ((F3DStatus.renderflags&F3D_RENDER_USE_TEXTUREMAPS) && texture!=0 && texture->bmp!=0) {
								if (cp!=F3DStatus.transparent_key_color) { //not the transparent color
									ib=(cp>>0x10)&0x000000ff;
									ig=(cp>>8)&0x000000ff;
									ir=cp&0x000000ff;
								}
							} else {  
								//NOT USING LIGHTING AND NOT USING TEXTURE MAPS
								cp=pbuf[i].c[j];
								ir=(float)((cp>>0x10)&0x000000ff);
								ig=(float)((cp>>8)&0x000000ff);
								ib=(float)(cp&0x000000ff);
							
							} //if (F3DStatus.renderflags&F3D_RENDER_USE_TEXTUREMAPS) {
						} //if (F3DStatus.renderflags&F3D_RENDER_USE_LIGHTING) {

						//add change in color between each point
						//this is the second linear interpolation of the color of the inbetween-vertices points
						//and is over change in x
						r=(float)(((pbuf[i].c[j]&0x00ff0000)>>0x10)&0x000000ff)+mr;
						g=(float)((pbuf[i].c[j]&0x0000ff00)>>0x8)+mg;
						b=(float)(pbuf[i].c[j]&0x000000ff)+mb;

						if (!((F3DStatus.renderflags&F3D_RENDER_USE_TRANSPARENT_COLOR_KEY) && cp==F3DStatus.transparent_key_color)) { //not the transparent color
							//cp=ir<<16 | ig<<8 | ib; //tph- not sure why this is here
							if (F3DStatus.renderflags&F3D_RENDER_USE_TRANSPARENCY) {
								//if partial transparency is used:
								//the colors of all polygons are stored
								//after the frame is rendered
								//each transparency value is compared and the final pixel color determined
								//RStatus.tbuf[k][i].num=1;

								//add this pixel to the transparency+z buffer in z order closest to farthest
								//order as we go- but only in the transp buffer order[] array- 
								//otherwise we would need to move 4 variables for each pixel with a larger z

								//an error occurs here when the camera and model have a y=0

								/*
								curtransp=F3DStatus.tbuf[k][i].num;
								F3DStatus.tbuf[k][i].color[curtransp]=cp;
								F3DStatus.tbuf[k][i].transparency[curtransp]=(1.0-polygon.transparency); //note transp is 1-trans
								F3DStatus.tbuf[k][i].z[curtransp]=sz;
*/
								curtransp=tbuf->num;
								tbuf->color[curtransp]=cp;
								tbuf->transparency[curtransp]=(1.0-polygon.transparency); //note transp is 1-trans
								tbuf->z[curtransp]=sz;
								if (curtransp==0) {  //no pixels yet, just add to transp+z buffer
									//F3DStatus.tbuf[k][i].order[curtransp]=0;
									tbuf->order[curtransp]=0;
								} else {  //there is at least 1 pixel in transp-z buffer 
									//add this pixel by z with closest first
									count=0;
//									while (sz>F3DStatus.tbuf[k][i].z[F3DStatus.tbuf[k][i].order[count]] && count<curtransp) {
									while (sz>tbuf->z[tbuf->order[count]] && count<curtransp) {
										count++;
									} //while

									if (count<curtransp) { //need to move rest of list down
/*
										for(count2=F3DStatus.tbuf[k][i].num+1;count2>count;count2--) {
											F3DStatus.tbuf[k][i].order[count2]=F3DStatus.tbuf[k][i].order[count2-1];
										}//for(count2
*/
										for(count2=tbuf->num+1;count2>count;count2--) {
											tbuf->order[count2]=tbuf->order[count2-1];
										}//for(count2

									} //if (count<curtransp
									//add this pixel
									//F3DStatus.tbuf[k][i].order[count]=curtransp;
									tbuf->order[count]=curtransp;
								} //curtransp==0
								//F3DStatus.tbuf[k][i].num++;
								tbuf->num++;

								//now draw all polygons in z order based on transparancy value
								//for only 1 polygon, just copy the color*transp value
								if (tbuf->num==1) { //if (F3DStatus.tbuf[k][i].num==1) {
									curtranspval=tbuf->transparency[0];
									fib=(curtranspval)*(float)(tbuf->color[0]&0xff);
									fig=(curtranspval)*(float)((tbuf->color[0]&0xff00)>>0x8);
									fir=(curtranspval)*(float)((tbuf->color[0]&0xff0000)>>0x10);
/*
									curtranspval=F3DStatus.tbuf[k][i].transparency[0];
									fib=(curtranspval)*(float)(F3DStatus.tbuf[k][i].color[0]&0xff);
									fig=(curtranspval)*(float)((F3DStatus.tbuf[k][i].color[0]&0xff00)>>0x8);
									fir=(curtranspval)*(float)((F3DStatus.tbuf[k][i].color[0]&0xff0000)>>0x10);
*/
								} else {  //more than one polygon 
									//go through list from closest to farthest
									//add all transparency values together until >=1.0
									fir=0.0;
									fig=0.0;
									fib=0.0;
									totaltransp=0.0;
									//for (count=0;count<F3DStatus.tbuf[k][i].num && totaltransp<0.99;count++) {
									for (count=0;count<tbuf->num && totaltransp<0.99;count++) {
										//skip any after the total transp>=1.0
										zorder=tbuf->order[count]; //zorder=F3DStatus.tbuf[k][i].order[count];
										curtranspval=tbuf->transparency[zorder];//curtranspval=F3DStatus.tbuf[k][i].transparency[zorder];
										if (totaltransp+curtranspval>1.0) {
											curtranspval=0.99-totaltransp;  //only add what adds to 1.0
											//curtranspval=0.3;//1.001-totaltransp;  //only add what adds to 1.0 (1.01 just to put over top)
										}
										totaltransp+=curtranspval;
										//curcolor=F3DStatus.tbuf[k][i].color[zorder];
										curcolor=tbuf->color[zorder];
										fib+=(curtranspval)*(float)(curcolor&0xff);
										fig+=(curtranspval)*(float)((curcolor&0xff00)>>0x8);
										fir+=(curtranspval)*(float)((curcolor&0xff0000)>>0x10);
										//no need to clamp because even if rg or b is 0xff, as long as transp do not add to>1.0 everything should be ok-use 0.99 to be on the safe side
										//if (fib>255.0) {fib=255.0;}
										//if (fig>255.0) {fig=255.0;}
										//if (fir>255.0) {fir=255.0;}

									} //for (count=0;count<F3DStatus.tbuf[k][i].num && totaltransp<0.99;count++) {
								} //if (F3DStatus.tbuf[k][i].num==1) { //only 1 polygon on this pixel

								ib=(u8)fib;
								ig=(u8)fig;
								ir=(u8)fir;

	//get total transparency and normalize

	//average colors together based on normalized transparency
							} //if (F3DStatus.renderflags&F3D_RENDER_USE_TRANSPARENCY) {

							//set the final bitmap pixel color
							*(bmp+offset)=ib;
							*(bmp+offset+1)=ig;
							*(bmp+offset+2)=ir;

						} //if (!((F3DStatus.renderflags&F3D_RENDER_USE_TRANSPARENT_COLOR_KEY) && cp==F3DStatus.transparent_key_color)) { //not the transparent color


						if (F3DStatus.renderflags&F3D_RENDER_USE_Z_BUFFER) {
							//todo: in theory with transparency we don't need to save any z
							//if (!((F3DStatus.renderflags&F3D_RENDER_USE_TRANSPARENT_COLOR_KEY) && cp==F3DStatus.transparent_key_color) && (F3DStatus.zbuf[k][i]>sz || F3DStatus.zbuf[k][i]==0.0)) {
							if (!((F3DStatus.renderflags&F3D_RENDER_USE_TRANSPARENT_COLOR_KEY) && cp==F3DStatus.transparent_key_color) && (*zbuf>sz || *zbuf==0.0)) {
									*zbuf=sz; //F3DStatus.zbuf[k][i]=sz;
							} //if (cp!=F3DStatus.chromakey_color && (zbuf[k][i]>sz || zbuf[k][i]==0.0)) {
						} //if (F3DStatus.renderflags&F3D_RENDER_USE_Z_BUFFER) {

					} //if (!(F3DStatus.renderflags&F3D_RENDER_USE_Z_BUFFER) || *zbuf>sz || *zbuf==0.0) {  


					sz+=mz;  //we cannot just add the difference in z
					pbuf[i].u[j]+=du;  //update here, because part of the scan line of this poly may be hidden by a different poly in the zbuffer
					pbuf[i].v[j]+=dv;

				}  //for(k=pbuf[i].x[j];k<pbuf[i].x[j+1];k++) {
			}  //if (pbuf[i].x[j+1]!=pbuf[i].x[j]) { //same point
		}  //for(j=0;j<pbuf[i].numx;j=j+2) { //for each polygon edge
	}  //for(i=low;i<high;i++) { //for each scan line
//	F3DStatus.PolygonCount++; //to determine polygons/sec

} //if (F3DStatus.renderflags&F3D_RENDER_POLYGON_FACES) 

}  //void DrawPolygon(u8 *bmp,int w, int h, CLIPPOINT3D *newpnt, POLYGON polygon, TEXTUREMAP *texture,int upsidedown)


//Draw a line to a bitmap
void DrawLine(u8 *bmp,int w, int h, CLIPPOINT3D *newpnt, F3D_LINE line, TEXTUREMAP *texture,int upsidedown)
{
	//u32 k,l,offset;
	u32 i,j;//,low,high;
//	float ex,ey,ez,m,mx,my,mz,mu,mv;
//	float sx,sy,sz,rx,ry,du,dv;
//	float su,eu,sv,ev;
//	float fr,fg,fb;
	u32 p0,p1;
//	u32 cp;
//	u32 sc,ec,osc;  //start color end color
//	u32 ir,ig,ib;
	//float mr,mg,mb,r,g,b;
	float dx,dy,dxdy,dydx;
	BITMAPFILEHEADER *bmf,*tbmf;
	BITMAPINFOHEADER *bmi,*tbmi;
	i32 bwidth,bheight,bdepth,boffset;
//	u32 curcolor;
	float tx,ty;
	u8 *pixeloffset,*bmpdata,*tdata;

//#if TEXTUREMAP
	i32 twidth,theight,tdepth;
		//picwidth,picdepth
//#endif //TEXTUREMAP
	//float mc,ic;
//#if TRANSPARENCY 
//	float fir,fig,fib;  //for partial transparency, floating point values of color components
//	float totaltransp;
//	i32 count,curtransp,zorder,count2;//,transpskip,count3,zcount;
//	float curtranspval;//,curdepth,nontdepth,thisz;
//#endif //TRANSPARENCY
//	POLYLIST *pbuf;	
	int incdec;

//todo: pass width,height, depth and data buffer
	bmf=(BITMAPFILEHEADER *)bmp;
	bmi=(BITMAPINFOHEADER *)((u8 *)bmp+sizeof(BITMAPFILEHEADER));

	bwidth=bmi->biWidth;
	bheight=bmi->biHeight;
	bdepth=bmi->biBitCount;
	boffset=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);

	bmpdata=bmp+boffset; //pointer to bitmap data

	if ((F3DStatus.renderflags&F3D_RENDER_USE_TEXTUREMAPS) && texture!=0 && texture->bmp!=0) {
		//Get Texture Map dimensions and depth
		tbmf=texture->bmp;//(BITMAPFILEHEADER *)texture;
		tbmi=texture->bmpinfo;//(BITMAPINFOHEADER *)((u8 *)texture+sizeof(BITMAPFILEHEADER));
		tdata=texture->bmpdata;
		twidth=tbmi->biWidth;
		theight=tbmi->biHeight;
		tdepth=tbmi->biBitCount/8;
	}

	if (F3DStatus.renderflags&F3D_RENDER_POINTS_ONLY) {
		if (bmi->biBitCount==32) {
			for(i=0;i<2;i++) {  
				pixeloffset=(u8 *)(bmpdata+(unsigned int)newpnt[i].y*w*4+(unsigned int)newpnt[i].x*4);
				//memset(pixeloffset,0xff,3);
				memcpy(pixeloffset,&line.color[i],3);
			} 
		} else {  //presumes 24-bit
			for(i=0;i<2;i++) {  
				pixeloffset=(u8 *)(bmpdata+(unsigned int)newpnt[i].y*w*3+(unsigned int)newpnt[i].x*3);
				//memset(pixeloffset,0xff,3);
				memcpy(pixeloffset,&line.color[i],3);
			} 
		} //if (bmi->biBitCount==32) {
	} else { //if (F3DStatus.renderflags&F3D_RENDER_POINTS_ONLY) {

		for(i=0;i<1;i++) {  

			//determine the dx and dy
			p0=i;
			p1=i+1;

			dx=newpnt[p1].x-newpnt[p0].x; //note that point order is reversed
			dy=newpnt[p1].y-newpnt[p0].y;
			if (fabs(dx)>fabs(dy)) {  //change in x is greater than change in y, so increment along x
				dydx=dy/fabs(dx); //dx will never be 0 because fabs(dx)>fabs(dy) 
				ty=newpnt[p0].y;
				if (dx>0) {  //dx>0
					incdec=1;
				} else {
					incdec=-1;
				}
				if (bmi->biBitCount==32) {
					for(j=(u32)newpnt[p0].x;j!=(u32)newpnt[p1].x;j+=incdec) {
						pixeloffset=(u8 *)(bmpdata+(unsigned int)ty*w*4+4*j);
						//memset(pixeloffset,0xff,3);
						memcpy(pixeloffset,&line.color[i],3);
						ty+=dydx;
					}
				} else {
					for(j=(u32)newpnt[p0].x;j!=(u32)newpnt[p1].x;j+=incdec) {
						pixeloffset=(u8 *)(bmpdata+(unsigned int)ty*w*3+3*j);
						//memset(pixeloffset,0xff,3);
						memcpy(pixeloffset,&line.color[i],3);
						ty+=dydx;
					}					
				} //if (bmi->biBitCount==32) {
			} else { //change in y is greater than change in x, so increment along y
				if (dy==0) { //can only happen if two points in a line are identical- just to avoid a crash
					dxdy=0.0; 
				} else {
					dxdy=dx/fabs(dy);  //dy will never = 0 because there is no way dx==dy==0 except for two identical points
				}
				tx=newpnt[p0].x;
				if (dy>=0) {
					incdec=1;
				} else {
					incdec=-1;
				}
				if (bmi->biBitCount==32) {
					for(j=(u32)newpnt[p0].y;j!=(u32)newpnt[p1].y;j+=incdec) {
						pixeloffset=(u8 *)(bmpdata+j*w*4+(unsigned int)tx*4);
						//memset(pixeloffset,0xff,3);
						memcpy(pixeloffset,&line.color[i],3);
						tx+=dxdy;
					}
				} else {
					for(j=(u32)newpnt[p0].y;j!=(u32)newpnt[p1].y;j+=incdec) {
						pixeloffset=(u8 *)(bmpdata+j*w*3+(unsigned int)tx*3);
						//memset(pixeloffset,0xff,3);
						memcpy(pixeloffset,&line.color[i],3);
						tx+=dxdy;
					}
				} //if (bmi->biBitCount==32) {
			} //if (fabs(dx)>fabs(dy)) {  //change in x is greater than change in y, so increment along x

		} //for(i=0;i<2;i++) {  
	} //if (F3DStatus.renderflags&F3D_RENDER_POINTS_ONLY) {

}  //void DrawLine(u8 *bmp,int w, int h, CLIPPOINT3D *newpnt, F3D_LINE line, TEXTUREMAP *texture,int upsidedown)



//Multiply a model's points by the model's axes (which have presumably changed from their original settings)
//this also applies the axis of any superaxes to the points
//void PointMatrix3DMultiplyAllAxes(MODEL *lmodel)
void TransformModelByAxes(MODEL *lmodel)
{
	u32 i, j;
	POINT3D *p, tp,*n,tn;
	//CLIPPOINT3D *p2;
	POINT3D *p2,*n2;
	MODELAXIS3D *axis, *supaxis;
	i32 numaxis, numaxis2, numpoint, numpoint2;
	MODELAXIS3D *modelaxis, *modelaxis2;
	POINT3D diff;


	p = lmodel->point;
	p2 = lmodel->apoint;
	n = lmodel->normal;
	n2 = lmodel->anormal;


	//todo: possibly add list of points on axis- because currently, I need to go through each point

	//was: for models with more than one axis, apply axis 0 to all points
	//is now: first apply local axes. Models with more than 1 axis all have axes with axis 0 as their first superaxis

	//go through all points in the model
	for (i = 0; i<lmodel->numpoint; i++)
	{
		//all points belong to some axis
		//axis=lmodel->axis[p[i].axis];  //get axis for this point
		axis = &lmodel->axis[lmodel->pointaxis[i]];  //get axis for this point
		//tp = p[i];
		memcpy(&tp, &p[i], sizeof(POINT3D));

		//apply the body segment rotation matrix to the point
		//todo: make all points relative to axis origin, and then apply the axis translation
		//move point to be relative to axis origin (not 0,0,0)
		//tp.x -= axis->loc.x;
		//tp.y -= axis->loc.y;
		//tp.z -= axis->loc.z;

		p2[i].x = tp.x*axis->m.x[0] + tp.y*axis->m.y[0] + tp.z*axis->m.z[0];// + axis.t[0];
		p2[i].y = tp.x*axis->m.x[1] + tp.y*axis->m.y[1] + tp.z*axis->m.z[1];// + axis.t[1];
		p2[i].z = tp.x*axis->m.x[2] + tp.y*axis->m.y[2] + tp.z*axis->m.z[2];// + axis.t[2];
																			
		//move point to axis location on model
		p2[i].x += axis->loc.x;
		p2[i].y += axis->loc.y;
		p2[i].z += axis->loc.z;


#if 0 //APPLY SUPERAXES
		//now apply the axis of all superaxes, in order
		//j=0;
		//while(axis->superaxis[j]!=-1 && j<MAXSUPERAXES) {
		for (j = 0; j < axis->numsuperaxis; j++) {
			supaxis = &lmodel->axis[axis->superaxis[j]];
			p2[i].x -= supaxis->loc.x;
			p2[i].y -= supaxis->loc.y;
			p2[i].z -= supaxis->loc.z;

			tp.x = p2[i].x;
			tp.y = p2[i].y;
			tp.z = p2[i].z;

			p2[i].x = tp.x*supaxis->m.x[0] + tp.y*supaxis->m.y[0] + tp.z*supaxis->m.z[0];
			p2[i].y = tp.x*supaxis->m.x[1] + tp.y*supaxis->m.y[1] + tp.z*supaxis->m.z[1];
			p2[i].z = tp.x*supaxis->m.x[2] + tp.y*supaxis->m.y[2] + tp.z*supaxis->m.z[2];

			p2[i].x += supaxis->loc.x;
			p2[i].y += supaxis->loc.y;
			p2[i].z += supaxis->loc.z;
			//j++;		
		} //for j
		  //} //while(axis.superaxis[j]!=0 && j<MAXSUPERAXES			
#endif //APPLY SUPERAXES
		  /*
		  //note that this is currently done by making axis 0 the first superaxis of all other axes
		  //
		  //now apply axis 0, if more than 1 axis
		  //axis 0 is the main axis of the model
		  //this is just for now - later determine how to apply axis 0 to the other axes
		  //perhaps just above
		  //#if 0
		  if (lmodel->numaxis>1) {
		  taxis=lmodel->axis[0];

		  p2[i].x-=taxis.loc.x;
		  p2[i].y-=taxis.loc.y;
		  p2[i].z-=taxis.loc.z;

		  tp.x =p2[i].x;
		  tp.y =p2[i].y;
		  tp.z =p2[i].z;

		  p2[i].x=tp.x*taxis.m.x[0] + tp.y*taxis.m.y[0] + tp.z*taxis.m.z[0];// + taxis.t[0];
		  p2[i].y=tp.x*taxis.m.x[1] + tp.y*taxis.m.y[1] + tp.z*taxis.m.z[1];// + taxis.t[1];
		  p2[i].z=tp.x*taxis.m.x[2] + tp.y*taxis.m.y[2] + tp.z*taxis.m.z[2];// + taxis.t[2];

		  p2[i].x+=taxis.loc.x;
		  p2[i].y+=taxis.loc.y;
		  p2[i].z+=taxis.loc.z;
		  } //if model->numaxis>1
		  //#endif
		  */


		  //TRANSFORM POINT NORMALS (for lighting)
			memcpy(&tn, &n[i], sizeof(POINT3D));

			n2[i].x = tn.x*axis->m.x[0] + tn.y*axis->m.y[0] + tn.z*axis->m.z[0];// + axis.t[0];
			n2[i].y = tn.x*axis->m.x[1] + tn.y*axis->m.y[1] + tn.z*axis->m.z[1];// + axis.t[1];
			n2[i].z = tn.x*axis->m.x[2] + tn.y*axis->m.y[2] + tn.z*axis->m.z[2];// + axis.t[2];


		  /*
		  //offset by model location - user can change with arrow keys
		  p2[i].x+=lmodel->location.x;
		  p2[i].y+=lmodel->location.y;
		  p2[i].z+=lmodel->location.z;
		  */

	} //	for(i=0;i<lmodel->numpoint;i++) //all points in model

	  //Using Joints saves time in applying many superaxis rotations (for example head needs to have rotation matrix of foot, lower leg, upper leg, and torso. With joints each axis only needs their accelerometer rotation and then higher axis number axis points are just moved so that the joint points coincide. But in practice- it is somewhat complicated because the torso, for example, has 5 joints and so it's impossible to align them all.)

	  //#if 0 //joint code
	  //for each axis apply the axis rotation to all joint points on the axis
	//possibly just go through all joints, get points and apply axis- since we ignore any joint points not listed as a joint- but in this case all joint points are used
	for (i = 1; i<lmodel->numaxis; i++) {
		axis = &lmodel->axis[i]; //get individual axis
		if (axis->numjointpoint>0) {
			for (j = 0; j<axis->numjointpoint; j++) {
				memcpy(&tp, &axis->jp[j], sizeof(POINT3D));
				p2 = &axis->ajp[j];

				//jointpoints are relative to axis local origin, which is 0,0,0
				//so no need to transform to apply axis rotation
				//move joint point to be relative to axis origin (not 0,0,0)
				//tp.x -= axis->loc.x;
				//tp.y -= axis->loc.y;
				//tp.z -= axis->loc.z;

				//transform (rotate) joint point by axis matrix
				p2->x = tp.x*axis->m.x[0] + tp.y*axis->m.y[0] + tp.z*axis->m.z[0];
				p2->y = tp.x*axis->m.x[1] + tp.y*axis->m.y[1] + tp.z*axis->m.z[1];
				p2->z = tp.x*axis->m.x[2] + tp.y*axis->m.y[2] + tp.z*axis->m.z[2];
				
				//move joint point to axis location on model
				p2->x += axis->loc.x;
				p2->y += axis->loc.y;
				p2->z += axis->loc.z;

			} //for(j=0;j<axis->numjoint;j++) {
		} //if (axis->numjoint>0) {

	} //for(i=0;i<lmodel->numaxis;i++) {  

//#if 0  
	  //move second jointpoint to first jointpoint
	  //for all model joints, move all points on the axis with the second joint point, so the second and first joint points are the same
	for (i = 0; i<lmodel->numjoint; i++) {
	//for (i = 0; i<4; i++) {
		//get difference between two joint points
		numaxis = lmodel->joint[i].a[0]; //get index of axis 1
		numaxis2 = lmodel->joint[i].a[1]; //get index of axis 2
		modelaxis = &lmodel->axis[numaxis]; //get axis 1
		modelaxis2 = &lmodel->axis[numaxis2]; //get axis 2
		numpoint = lmodel->joint[i].p[0]; //get index of joint point 1
		numpoint2 = lmodel->joint[i].p[1]; //get index of joint point 2
		p = &modelaxis->ajp[numpoint]; //get joint point 1
		p2 = &modelaxis2->ajp[numpoint2]; //get joint point 2
		diff.x = p2->x - p->x;
		diff.y = p2->y - p->y;
		diff.z = p2->z - p->z;

		//now move all points on the second axis by the difference between the two joint points
		for (j = 0; j<modelaxis2->numpoint; j++) {
			p = &lmodel->apoint[modelaxis2->p[j]]; //get a pointer to each individual point on the second axis

												   //transform (translate) joint point by difference between two joints (first joint is viewed as stationary/secdon joint points are moved to first axis)
			p->x -= diff.x;
			p->y -= diff.y;
			p->z -= diff.z;

		} //for(j=0;j<axis2->numpoint;j++) {

		//now move all transformed joint points on the moved (second) axis by the same offset, so that later translations will be accurate
	  for (j = 0; j<modelaxis2->numjointpoint; j++) {
		p = &modelaxis2->ajp[j]; //get joint point
		p->x -= diff.x;
		p->y -= diff.y;
		p->z -= diff.z;
		} //for j

	} //for(i=0;i<lmodel->numjoint;i++) {  
//#endif


  //transform polygon normals (todo:only if using flat shading or polygon has use polygon normal flag set)- 
  //one problem is that you don't know if the use polygon normal is set during gouraud shading-perhaps that should not apply during gouraud shading
  //	if (((F3DStatus.renderflags&F3D_RENDER_USE_LIGHTING) && //(F3DStatus.renderflags&F3D_RENDER_FLAT_SHADING))) {		
  //	}
  //#if 0 //TRANSFORM POLYGON NORMALS
	for (i = 0; i<lmodel->numpolygon; i++) {
		axis = &lmodel->axis[lmodel->polygon[i].axis];  //get axis for this point
		p = &lmodel->polygon[i].n;
		p2 = &lmodel->polygon[i].an;

		p2->x = p->x*axis->m.x[0] + p->y*axis->m.y[0] + p->z*axis->m.z[0];
		p2->y = p->x*axis->m.x[1] + p->y*axis->m.y[1] + p->z*axis->m.z[1];
		p2->z = p->x*axis->m.x[2] + p->y*axis->m.y[2] + p->z*axis->m.z[2];

#if 0 
		//now apply the axis of all superaxes, in order
		for (j = 0; j< axis->numsuperaxis; j++) {
			supaxis = &lmodel->axis[axis->superaxis[j]];
			tp.x = p2->x;
			tp.y = p2->y;
			tp.z = p2->z;

			p2->x = tp.x*supaxis->m.x[0] + tp.y*supaxis->m.y[0] + tp.z*supaxis->m.z[0];
			p2->y = tp.x*supaxis->m.x[1] + tp.y*supaxis->m.y[1] + tp.z*supaxis->m.z[1];
			p2->z = tp.x*supaxis->m.x[2] + tp.y*supaxis->m.y[2] + tp.z*supaxis->m.z[2];

		} //		for (j = 0;j< axis->numsuperaxis; j++) {
#endif

	} //for i
	  //#endif //TRANSFORM NORMALS

	  //#if 0 
	  //apply model axis 0 if there is more than one axis- currently axis 0 is the axis for the entire model
	if (lmodel->numaxis>1) {
		axis = &lmodel->axis[0];
		p2 = lmodel->apoint;


		for (i = 0; i < lmodel->numpoint; i++) { //all points in model

			memcpy(&tp, &p2[i], sizeof(POINT3D));
			//apply the body segment rotation matrix to the point
			//note that apparently I am not using axis->loc currently but instead using model->location to move the models around
			//			tp.x -= axis->loc.x;
			//			tp.y -= axis->loc.y;
			//			tp.z -= axis->loc.z;

			p2[i].x = tp.x*axis->m.x[0] + tp.y*axis->m.y[0] + tp.z*axis->m.z[0];// + axis.t[0];
			p2[i].y = tp.x*axis->m.x[1] + tp.y*axis->m.y[1] + tp.z*axis->m.z[1];// + axis.t[1];
			p2[i].z = tp.x*axis->m.x[2] + tp.y*axis->m.y[2] + tp.z*axis->m.z[2];// + axis.t[2];

																				//p2[i].x += axis->loc.x;
																				//p2[i].y += axis->loc.y;
																				//p2[i].z += axis->loc.z;

		} //for i
	} //if (lmodel->numaxis>1) {
	  //#endif 

	p2 = lmodel->apoint;
	for (i = 0; i<lmodel->numpoint; i++) { //all points in model

										   //offset by model location - user can change with arrow keys
		p2[i].x += lmodel->location.x;
		p2[i].y += lmodel->location.y;
		p2[i].z += lmodel->location.z;
	} //	for(i=0;i<lmodel->numpoint;i++) //all points in model



}  //TransformModelByAxes(lmodel);



//multiply model points by an axis (for example a camera)
//void ClipPointMatrix3DMultiply(CLIPPOINT3D *lpnt,AXIS3D axis,POINT3D axisloc,CLIPPOINT3D *newpnt,int numpnt)
void PointsMatrix3DMultiplyToClipPoints(POINT3D *lpnt,AXIS3D axis,POINT3D axisloc,CLIPPOINT3D *newpnt,int numpnt)
{
int i;
POINT3D tp;
//need a temp point because if newpnt=pnt pnt will change when newpnt does
//z axis is reversed for viewer (if viewer is at -200 = add 200 to the model)

  for(i=0;i<numpnt;i++)
    {
	//add camera translation
		tp.x = lpnt[i].x -axisloc.x;
		tp.y = lpnt[i].y -axisloc.y;
		tp.z = lpnt[i].z -axisloc.z;

	//then rotate
	newpnt[i].x = tp.x*axis.x[0] + tp.y*axis.y[0] + tp.z*axis.z[0];  
	newpnt[i].y = tp.x*axis.x[1] + tp.y*axis.y[1] + tp.z*axis.z[1];
	newpnt[i].z = tp.x*axis.x[2] + tp.y*axis.y[2] + tp.z*axis.z[2];
	}
} //void PointsMatrix3DMultiplyToClipPoints(POINT3D *lpnt,AXIS4D axis, CLIPPOINT3D *newpnt, int numpnt)


//Determine if a point is clipped by a rectilinear volume, and set the corresponding bits in an int variable
unsigned int IsPointClipped(CLIPPOINT3D p)
{
unsigned int flag=0;
float tz;
float w,h;

w=F3DStatus.WinMaxX/2.0;  //probably should be full width/2 or width-2/2
h=F3DStatus.WinMaxY/2.0;

//clip Z
if (p.z <= 0.0)
	flag |= POLY_CLIP_FRONT;

//clip X
	if (p.x > w)  flag |= POLY_CLIP_RIGHT;
	if (p.x < -w) flag |= POLY_CLIP_LEFT;

//clip Y
	if (p.y > h)  flag |= POLY_CLIP_TOP;
	if (p.y < -h) flag |= POLY_CLIP_BOTTOM;

	return flag;
}  //unsigned int IsPointClipped(CLIPPOINT3D p,float w,float h)


//Determine if a point is clipped to a volume defined by perspective, and set the corresponding bits in an int variable
unsigned int IsPointClippedPerspective(CLIPPOINT3D p)
{
unsigned int flag=0;
float tz;

//clip Z
if (p.z <= 0.0)
	flag |= POLY_CLIP_FRONT;

//clip X
	tz = F3DStatus.tanFOV2x * p.z;
	if (p.x > tz)  flag |= POLY_CLIP_RIGHT;
	if (p.x < -tz) flag |= POLY_CLIP_LEFT;


//clip Y
	tz = F3DStatus.tanFOV2y * p.z;
	if (p.y > tz)  flag |= POLY_CLIP_TOP;
	if (p.y < -tz) flag |= POLY_CLIP_BOTTOM;

	return flag;
}  //unsigned int IsPointClipped(CLIPPOINT3D p)


//Determine if a polygon is totally, partially, or not clipped
int IsPolygonOrLineClipped(CLIPPOINT3D *lpnt,int nump,unsigned int *PolyFlag)
{	
	int i;
	unsigned int and_flags, or_flags;

	and_flags=~(unsigned int)0;
	or_flags = 0;

	//for all vertices set clip flags
	for(i=0;i<nump;i++)
	{
	unsigned int TempFlag;
    TempFlag = lpnt[i].Flag;
	or_flags  |= TempFlag;
	and_flags &= TempFlag;
	}

	*PolyFlag &=~P_CLIPFLAG;
	*PolyFlag |= or_flags;
	return(and_flags?TOTAL_CLIP:(or_flags?PARTIAL_CLIP:NO_CLIP));
} //int IsPolygonClipped(CLIPPOINT3D *lpnt,int nump,unsigned int *PolyFlag)

int ClipPolygon(int NumVertices)
{

	if (F3DStatus.PolyFlag & POLY_CLIP_FRONT)
		NumVertices = ClipToPlane(NumVertices, WithinFront);
		//if (PolyFlag & POLY_CLIP_BACK) //clips forward Z (yon plane)
		//	NumVertices = ClipToPlane(NumVertices, WithinBack);

	if (F3DStatus.renderflags&F3D_RENDER_PERSPECTIVE) {
		if (F3DStatus.PolyFlag & POLY_CLIP_LEFT)
			NumVertices = ClipToPlane(NumVertices,WithinLeftFov);
		if (F3DStatus.PolyFlag & POLY_CLIP_RIGHT)
			NumVertices = ClipToPlane(NumVertices,WithinRightFov);
		if (F3DStatus.PolyFlag & POLY_CLIP_TOP)
			NumVertices = ClipToPlane(NumVertices,WithinTopFov);
		if (F3DStatus.PolyFlag & POLY_CLIP_BOTTOM)
			NumVertices = ClipToPlane(NumVertices,WithinBottomFov);
	} else {
		if (F3DStatus.PolyFlag & POLY_CLIP_LEFT)
			NumVertices = ClipToPlane(NumVertices,WithinLeftRect);
		if (F3DStatus.PolyFlag & POLY_CLIP_RIGHT)
			NumVertices = ClipToPlane(NumVertices,WithinRightRect);
		if (F3DStatus.PolyFlag & POLY_CLIP_TOP)
			NumVertices = ClipToPlane(NumVertices,WithinTopRect);
		if (F3DStatus.PolyFlag & POLY_CLIP_BOTTOM)
			NumVertices = ClipToPlane(NumVertices,WithinBottomRect);	
	} //if (F3DStatus.renderflags&F3D_RENDER_PERSPECTIVE) {
	return(NumVertices);
}

int ClipLine()
{

	if (F3DStatus.LineFlag & POLY_CLIP_FRONT)
		ClipLineToPlane(WithinFront);
	//if (PolyFlag & POLY_CLIP_BACK) //clips forward Z (yon plane)
	//	NumVertices = ClipToPlane(NumVertices, WithinBack);
	if (F3DStatus.renderflags&F3D_RENDER_PERSPECTIVE) {
		if (F3DStatus.LineFlag & POLY_CLIP_LEFT)
			//ClipToPlane(2,WithinLeftFov);
			ClipLineToPlane(WithinLeftFov);
		if (F3DStatus.LineFlag & POLY_CLIP_RIGHT)
			//ClipToPlane(2,WithinRightFov);
			ClipLineToPlane(WithinRightFov);
		if (F3DStatus.LineFlag & POLY_CLIP_TOP)
			ClipLineToPlane(WithinTopFov);
			//ClipToPlane(2,WithinTopFov);
		if (F3DStatus.LineFlag & POLY_CLIP_BOTTOM)
			ClipLineToPlane(WithinBottomFov);
			//ClipToPlane(2,WithinBottomFov);
	} else {
		if (F3DStatus.LineFlag & POLY_CLIP_LEFT)
			ClipLineToPlane(WithinLeftRect);
		if (F3DStatus.LineFlag & POLY_CLIP_RIGHT)
			ClipLineToPlane(WithinRightRect);
		if (F3DStatus.LineFlag & POLY_CLIP_TOP)
			ClipLineToPlane(WithinTopRect);
		if (F3DStatus.LineFlag & POLY_CLIP_BOTTOM)
			ClipLineToPlane(WithinBottomRect);
	} //if (F3DStatus.renderflags&F3D_RENDER_PERSPECTIVE) {
	return(1);
}

//F3DStatus.tanFOV2x*F3DStatus.clippoint[v].z is the length (in the x dimension) of the (visible) perpective volume at clippoint.z
//so the clippoint.x must be > -length to be within the left, and clippoint.x must be < length to be with the right
float WithinFront(int v)
{ return((float)(F3DStatus.clippoint[v].z - 0.0));}
float WithinBack(int v)
{ return((float)(0.0-F3DStatus.clippoint[v].z));}
float WithinLeft(int v)
{ return((float)(F3DStatus.clippoint[v].z + F3DStatus.clippoint[v].x));}
float WithinLeftFov(int v)
{ return((float)(F3DStatus.tanFOV2x * F3DStatus.clippoint[v].z + F3DStatus.clippoint[v].x));}
float WithinRight(int v)
{ return((float)(F3DStatus.clippoint[v].z - F3DStatus.clippoint[v].x));}
float WithinRightFov(int v)
{ return((float)(F3DStatus.tanFOV2x * F3DStatus.clippoint[v].z - F3DStatus.clippoint[v].x));}
float WithinBottom(int v)
{ return((float)(F3DStatus.clippoint[v].z + F3DStatus.clippoint[v].y));}
float WithinBottomFov(int v)
{ return((float)(F3DStatus.tanFOV2y * F3DStatus.clippoint[v].z + F3DStatus.clippoint[v].y));}
float WithinTop(int v)
{ return((float)(F3DStatus.clippoint[v].z - F3DStatus.clippoint[v].y));}
float WithinTopFov(int v)
{ return((float)(F3DStatus.tanFOV2y * F3DStatus.clippoint[v].z - F3DStatus.clippoint[v].y));}

float WithinLeftRect(int v)
{ return(F3DStatus.WinMaxX/2.0 + F3DStatus.clippoint[v].x);}
float WithinRightRect(int v)
{ return(F3DStatus.WinMaxX/2.0 - F3DStatus.clippoint[v].x);}
float WithinBottomRect(int v)
{ return(F3DStatus.WinMaxY/2.0 + F3DStatus.clippoint[v].y);}
float WithinTopRect(int v)
{ return(F3DStatus.WinMaxY/2.0 - F3DStatus.clippoint[v].y);}



	float prv_within,cur_within,t;
	int prv,cur,newn;
	char prv_out,cur_out;

int ClipToPlane(int oldn, WithinFunc *within)
{
	unsigned int r[3],g[3],b[3];

	prv_within=0.0,cur_within=0.0,t=0.0;
	prv=0,cur=0,newn=0;
	prv_out=0,cur_out=0;

	if (!oldn) return 0;

	prv = oldn-1;
	prv_within = (*within) (prv);
	prv_out = prv_within < 0;	

	while (oldn--)
	{
		cur_within = (*within) (cur);
		cur_out = cur_within < 0;
		//if ((cur_out^prv_out) && cur_within) {
		if (cur_out^prv_out) {
			//either the current point or the previous point were out, 
			//and the && cur_within prevents a problem when the current point is a fraction over the boundary

			//note that this only works for clockwise vertices (presumes the previous point is larger than the current point)
			t = prv_within / (prv_within - cur_within);
				
			#define Interpolate(field) \
			 (F3DStatus.newpoint[newn].field=F3DStatus.clippoint[prv].field + (F3DStatus.clippoint[cur].field-F3DStatus.clippoint[prv].field)*t)

			Interpolate(u);
			Interpolate(v);
			Interpolate(x);
			Interpolate(y);
			Interpolate(z);

			//todo: below only matters if lighting is enabled?
			//Interpolate color components
			r[0]=(F3DStatus.clippoint[prv].color&0x00ff0000)>>16;
			r[1]=(F3DStatus.clippoint[cur].color&0x00ff0000)>>16;
			if (r[0]>r[1]) {
				r[2]=r[0]-(r[0]-r[1])*t;
			} else {
				r[2]=r[0]+(r[1]-r[0])*t;
			}
			g[0]=(F3DStatus.clippoint[prv].color&0x0000ff00)>>8;
			g[1]=(F3DStatus.clippoint[cur].color&0x0000ff00)>>8;
			if (g[0]>g[1]) {
				g[2]=g[0]-(g[0]-g[1])*t;
			} else {
				g[2]=g[0]+(g[1]-g[0])*t;
			}
			b[0]=(F3DStatus.clippoint[prv].color&0x000000ff);
			b[1]=(F3DStatus.clippoint[cur].color&0x000000ff);
			if (b[0]>b[1]) {
				b[2]=b[0]-(b[0]-b[1])*t;
			} else {
				b[2]=b[0]+(b[1]-b[0])*t;
			}
			F3DStatus.newpoint[newn].color=(r[2]<<16) | (g[2]<<8) | b[2];

			#undef Interpolate
			
			newn++;
		} //		if (cur_out^prv_out) {			

		if (!cur_out) 
		{	
			F3DStatus.newpoint[newn] = F3DStatus.clippoint[cur];
			newn++;  
		}
		prv = cur;
		prv_within = cur_within;
		prv_out = cur_out;
		cur++;  
	}  //end while 

	memcpy(F3DStatus.clippoint,F3DStatus.newpoint,sizeof(CLIPPOINT3D)*newn);
	return newn; //number of new points
}	    //ClipToPlane

int ClipLineToPlane(WithinFunc *within)
{
	float temp;
	unsigned int r[3],g[3],b[3];

	prv_within=0.0,cur_within=0.0,t=0.0;
	prv=0,cur=1;
	prv_out=0,cur_out=0;

	//determine which point is outside the boundary
	prv = 0;
	prv_within = (*within) (prv);
	prv_out = prv_within < 0;	

	cur_within = (*within) (cur);
	cur_out = cur_within < 0;

	if (cur_out) {
	//if (!prv_out) {
		//point 1 is out- switch points
		temp=cur_within;
		cur_within=prv_within;
		prv_within=temp;
		prv=1;
		cur=0;
	} 


//	if (cur_out^prv_out) {  //occurs everytime with clipped line
		//either the current point or the previous point were out, 

		t = prv_within / (prv_within - cur_within);  //determine what fraction of the line is outside the visible volume (past the tangent line)

	#define Interpolate(field) \
		(F3DStatus.clippoint[prv].field=F3DStatus.clippoint[prv].field + (F3DStatus.clippoint[cur].field-F3DStatus.clippoint[prv].field)*t)

		Interpolate(u);
		Interpolate(v);
		Interpolate(x);
		Interpolate(y);
		Interpolate(z);

		//todo: below only matters if lighting is enabled?
		//Interpolate color components
		r[0]=(F3DStatus.clippoint[prv].color&0x00ff0000)>>16;
		r[1]=(F3DStatus.clippoint[cur].color&0x00ff0000)>>16;
		if (r[0]>r[1]) {
			r[2]=r[0]-(r[0]-r[1])*t;
		} else {
			r[2]=r[0]+(r[1]-r[0])*t;
		}
		g[0]=(F3DStatus.clippoint[prv].color&0x0000ff00)>>8;
		g[1]=(F3DStatus.clippoint[cur].color&0x0000ff00)>>8;
		if (g[0]>g[1]) {
			g[2]=g[0]-(g[0]-g[1])*t;
		} else {
			g[2]=g[0]+(g[1]-g[0])*t;
		}
		b[0]=(F3DStatus.clippoint[prv].color&0x000000ff);
		b[1]=(F3DStatus.clippoint[cur].color&0x000000ff);
		if (b[0]>b[1]) {
			b[2]=b[0]-(b[0]-b[1])*t;
		} else {
			b[2]=b[0]+(b[1]-b[0])*t;
		}
		F3DStatus.newpoint[prv].color=(r[2]<<16) | (g[2]<<8) | b[2];

		#undef Interpolate



//	}//	if (cur_out^prv_out) {

	return 1; 
} //ClipLineToPlane


//Multiple a 3x3 matrix
//m3=m1 x m2
//m3 because we may not want to change the original matrix (m1)
//could add num= number of axes for multiple axes
void MatrixMultiply3D(AXIS3D *m1,AXIS3D *m2,AXIS3D *m3)
{
	AXIS3D  tempaxis;

	tempaxis.x[0]=m1->x[0]*m2->x[0] + m1->x[1]*m2->y[0] + m1->x[2]*m2->z[0];
	tempaxis.x[1]=m1->x[0]*m2->x[1] + m1->x[1]*m2->y[1] + m1->x[2]*m2->z[1];
	tempaxis.x[2]=m1->x[0]*m2->x[2] + m1->x[1]*m2->y[2] + m1->x[2]*m2->z[2];

	tempaxis.y[0]=m1->y[0]*m2->x[0] + m1->y[1]*m2->y[0] + m1->y[2]*m2->z[0];
	tempaxis.y[1]=m1->y[0]*m2->x[1] + m1->y[1]*m2->y[1] + m1->y[2]*m2->z[1];
	tempaxis.y[2]=m1->y[0]*m2->x[2] + m1->y[1]*m2->y[2] + m1->y[2]*m2->z[2];

	tempaxis.z[0]=m1->z[0]*m2->x[0] + m1->z[1]*m2->y[0] + m1->z[2]*m2->z[0];
	tempaxis.z[1]=m1->z[0]*m2->x[1] + m1->z[1]*m2->y[1] + m1->z[2]*m2->z[1];
	tempaxis.z[2]=m1->z[0]*m2->x[2] + m1->z[1]*m2->y[2] + m1->z[2]*m2->z[2];

	memcpy(m3,&tempaxis,sizeof(AXIS3D));
	/*
	m1->x[0]=tempaxis.x[0];
	m1->x[1]=tempaxis.x[1];
	m1->x[2]=tempaxis.x[2];
	
	m1->y[0]=tempaxis.y[0];
	m1->y[1]=tempaxis.y[1];
	m1->y[2]=tempaxis.y[2];

	m1->z[0]=tempaxis.z[0];
	m1->z[1]=tempaxis.z[1];
	m1->z[2]=tempaxis.z[2];
	*/
} //void MatrixMultiply3D(AXIS3D *m1,AXIS3D *m2,AXIS3D *m3)


void AddCameraToScene(CAMERA *lcamera)
{
OBJLIST *lobjlist;

if (F3DStatus.iobjlist==0)
{
F3DStatus.iobjlist=(OBJLIST *)malloc(sizeof(OBJLIST));
memset(F3DStatus.iobjlist,0,sizeof(OBJLIST));
F3DStatus.iobjlist->type='c';
F3DStatus.iobjlist->camera=lcamera;
strcpy(F3DStatus.iobjlist->name,lcamera->name);
}
else
{  //there is at least 1 model in scene
	lobjlist=F3DStatus.iobjlist;
while (lobjlist!=0)
{
if (lobjlist->next==0)
{  
lobjlist->next=(OBJLIST *)malloc(sizeof(OBJLIST));
lobjlist=lobjlist->next;
memset(lobjlist,0,sizeof(OBJLIST));
lobjlist->type='c';
lobjlist->camera=lcamera;
strcpy(lobjlist->name,lcamera->name);
}
lobjlist=lobjlist->next;  //go to next object
}  //end while
}   //end if at least 1 motion playing
	
}  //AddCameraToScene


void RemoveCameraFromScene(CAMERA *lcamera)
{
OBJLIST *lobjlist,*last;

lobjlist=F3DStatus.iobjlist;  //top of list
last=F3DStatus.iobjlist;

while (lobjlist!=0)
{
if (lobjlist->camera==lcamera) {  //is a match
	if (lobjlist->next==0) { //is last in objlist
	    last->next=0;
		if (lobjlist==F3DStatus.iobjlist) {
			F3DStatus.iobjlist=0;  //and first
		}
	} else {
		if (lobjlist==F3DStatus.iobjlist) {
			F3DStatus.iobjlist=lobjlist->next;  //was first
		} else {
			last->next=lobjlist->next;
		}
	} //if (lobjlist->next==0) { //is last in objlist
	free(lobjlist);
	lobjlist=0; //exit loop
} else { //if there is a match
	last=lobjlist;
	lobjlist=lobjlist->next;
} //if (lobjlist->camera==lcamera) {  //is a match
}  //end while
}  //RemoveCameraFromScene


void RemoveAllCamerasFromScene(void)
{
CAMERA *lcamera;

lcamera=F3DStatus.icamera;
while(lcamera!=0) {
	lcamera=lcamera->next;
	free(F3DStatus.icamera);
	F3DStatus.icamera=lcamera;
}  //while
F3DStatus.icamera=0;
} //void RemoveAllCamerasFromScene(void)

void AddLightToScene(LIGHT *llight)
{
OBJLIST *lobjlist;

if (F3DStatus.iobjlist==0)
{
F3DStatus.iobjlist=(OBJLIST *)malloc(sizeof(OBJLIST));
memset(F3DStatus.iobjlist,0,sizeof(OBJLIST));
F3DStatus.iobjlist->type='l';
F3DStatus.iobjlist->light=llight;
strcpy(F3DStatus.iobjlist->name,llight->name);
}
else
{  //there is at least 1 model in scene
	lobjlist=F3DStatus.iobjlist;
while (lobjlist!=0)
{
if (lobjlist->next==0)
{  
lobjlist->next=(OBJLIST *)malloc(sizeof(OBJLIST));
lobjlist=lobjlist->next;
memset(lobjlist,0,sizeof(OBJLIST));
lobjlist->type='l';
lobjlist->light=llight;
strcpy(lobjlist->name,llight->name);
}
lobjlist=lobjlist->next;  //go to next object
}  //end while
}   //end if at least 1 motion playing
	
}  //AddLightToScene


void RemoveLightFromScene(LIGHT *llight)
{
OBJLIST *lobjlist,*last;

lobjlist=F3DStatus.iobjlist;  //top of list
last=F3DStatus.iobjlist;

while (lobjlist!=0) {
	if (lobjlist->light==llight) {  //is a match
		if (lobjlist->next==0) { //is last in objlist
			last->next=0;
			if (lobjlist==F3DStatus.iobjlist) {
				F3DStatus.iobjlist=0;  //and first
			}
		} else {
			if (lobjlist==F3DStatus.iobjlist) {
				F3DStatus.iobjlist=lobjlist->next;  //was first
			} else {
				last->next=lobjlist->next;
			}
		} //if (lobjlist->next==0) { //is last in objlist
		free(lobjlist);
		lobjlist=0; //exit loop
	} else { //if there is a match
		last=lobjlist;
		lobjlist=lobjlist->next;
	} //if (lobjlist->light==llight) {  //is a match
}  //end while
}  //RemoveLightFromScene

void RemoveAllLightsFromScene(void)
{
LIGHT *llight;

llight=F3DStatus.ilight;
while(llight!=0) {
	llight=llight->next;
	//free(ilight);
	RemoveLightFromScene(F3DStatus.ilight); //needs to be done this way to delete from objlist
	F3DStatus.ilight=llight;
}  //while
F3DStatus.ilight=0;
} //void RemoveAllLightsFromScene(void)

//Add a model to the list of all models the program has loaded
//presumes model is mallocd
int AddModelToModelList(MODEL *lmodel)
{
	MODEL *lmodel2;

	//check if already in list 
	lmodel2=F3DStatus.imodel;
	while(lmodel2!=0) {
		if (lmodel2==lmodel) {
			return(1);
		}
		lmodel2=(MODEL *)lmodel2->next;
	} //while

	if (F3DStatus.imodel==0) {
		//first model
		F3DStatus.imodel=lmodel;
		return(1);
	} else {
		//go through models in list and add to end
		lmodel2=F3DStatus.imodel;
		while(lmodel2->next!=0) {
			lmodel2=(MODEL *)lmodel2->next;
		}
		lmodel2->next=lmodel;
	} //if (F3DStatus.imodel==0) {
	return(1);
} //int AddModelToModelList(MODEL *lmodel)


//Remove a model from the list of all models the program has loaded
//doesn't free model space- FreeModel does that
int RemoveModelFromModelList(MODEL *lmodel)
{
	MODEL *lmodel2,*last;

	lmodel2=F3DStatus.imodel;
	last=0;
	while(lmodel2!=0) {
		if (lmodel2==lmodel) {
			if (last==0) {
				F3DStatus.imodel=lmodel->next;
				return(1);
			} else {
				last->next=lmodel->next;
				return(1);
			}//last==0
		} //if (lmodel2==lmodel) {
		last=lmodel2;
		lmodel2=lmodel2->next;
	} //while
	return(1);
} //int RemoveModelFromModelList(MODEL *lmodel)

//Add a model to the scene object list
//if no object list exists one is created
void AddModelToScene(MODEL *lmodel)
{
	OBJLIST *lobjlist,*iobjlist;

	iobjlist=F3DStatus.iobjlist;
	if (iobjlist==0) {
		iobjlist=(OBJLIST *)malloc(sizeof(OBJLIST));
		memset(iobjlist,0,sizeof(OBJLIST));
		iobjlist->type='m';
		iobjlist->model= lmodel;
		strcpy(iobjlist->name,lmodel->name);
		F3DStatus.iobjlist=iobjlist;
	} else {  
		//there is at least 1 model in scene
		lobjlist=iobjlist;
		while (lobjlist!=0) {
			if (lobjlist->model==lmodel) { //model is already in the scene
				return;
			}
			if (lobjlist->next==0) {
				lobjlist->next=(OBJLIST *)malloc(sizeof(OBJLIST));
				lobjlist=lobjlist->next;
				memset(lobjlist,0,sizeof(OBJLIST));
				lobjlist->type='m';
				lobjlist->model=lmodel;
				strcpy(lobjlist->name,lmodel->name);
			}
			lobjlist=lobjlist->next;  //go to next object
		}  //end while
	}   //end if at least 1 model in the scene
	
}  //AddModelToScene


void RemoveModelFromScene(MODEL *lmodel)
{
	OBJLIST *lobjlist,*last;

	lobjlist=F3DStatus.iobjlist;  //top of list
	last=F3DStatus.iobjlist;

	while (lobjlist!=0) {
		if (lobjlist->model==lmodel) {  //is a match
			if (lobjlist->next==0) { //is last in objlist
				last->next=0;
				if (lobjlist==F3DStatus.iobjlist) {
					F3DStatus.iobjlist=0;  //and first
				}
			} else {
				if (lobjlist==F3DStatus.iobjlist) {
					F3DStatus.iobjlist=lobjlist->next;  //was first
				} else {
					last->next=lobjlist->next;
				}
			} //if (lobjlist->next==0) { //is last in objlist
			free(lobjlist);
			lobjlist=0; //exit loop
		} else { //if there is a match
			last=lobjlist;
			lobjlist=lobjlist->next;
		} //if (lobjlist->model==lmodel) {  //is a match
	}  //end while
}  //RemoveModelFromScene(MODEL *lmodel)


void RemoveAllModelsFromScene(void)
{
MODEL *lmodel;

lmodel=F3DStatus.imodel;
while(lmodel!=0) {
	lmodel=lmodel->next;
	RemoveModelFromScene(F3DStatus.imodel);  //need to delete from objlist
	F3DStatus.imodel=lmodel;
}  //while
F3DStatus.imodel=0;
} //void RemoveAllModelsFromScene(void)

int FreeModel(MODEL *lmodel)
{
	int i;

//probably the code should see lmodel->numpoint>0, free if !=0, and set both lmodel->numpoint and lmodel->apoint=0 after freeing

	//models
	if (lmodel!=0) {
		if (lmodel->point!=0) {
			free(lmodel->point);
			lmodel->point=0;
		} //if (lmodel->point!=0) {
		if (lmodel->apoint!=0) {
			free(lmodel->apoint);
			lmodel->apoint=0;
		} //if (lmodel->apoint!=0) {
		if (lmodel->cpoint!=0) {
			free(lmodel->cpoint);
			lmodel->cpoint=0;
		} //if (lmodel->cpoint!=0) {
		if (lmodel->normal!=0) {
			free(lmodel->normal);
			lmodel->normal=0;
		} //if (lmodel->normal!=0) {
		if (lmodel->anormal!=0) {
			free(lmodel->anormal);
			lmodel->anormal=0;
		} //if (lmodel->anormal!=0) {
		if (lmodel->pointaxis!=0) {
			free(lmodel->pointaxis);
			lmodel->pointaxis=0;
		} //if (lmodel->cpoint!=0) {
		if (lmodel->line!=0) {
			free(lmodel->line); //causes crash
			lmodel->line=0;
		} //if (lmodel->polygon!=0) {
		if (lmodel->polygon!=0) {
			free(lmodel->polygon);
			lmodel->polygon=0;
		} //if (lmodel->polygon!=0) {
		if (lmodel->axis!=0) {
			//free all superaxe, joint points, and index to points on the joint
			for (i = 0; i < lmodel->numaxis; i++) {
				if (lmodel->axis[i].numsuperaxis > 0 && lmodel->axis[i].superaxis!=0) {
					free(lmodel->axis[i].superaxis);
				}
				if (lmodel->axis[i].numjointpoint > 0 && lmodel->axis[i].jp!=0 && lmodel->axis[i].ajp!=0) {
					free(lmodel->axis[i].jp);
					free(lmodel->axis[i].ajp);
				}
				if (lmodel->axis[i].numpoint > 0 && lmodel->axis[i].p!=0) {
					free(lmodel->axis[i].p);
				}
			} //for i
			free(lmodel->axis);
			lmodel->axis=0;
		} //if (lmodel->axis!=0) {
		if (lmodel->joint!=0) {
			free(lmodel->joint);
			lmodel->joint=0;
		} //if (lmodel->axis!=0) {

		if (lmodel->texture!=0) {
			free(lmodel->texture);
			lmodel->texture=0;
		} //if (lmodel->texture!=0) {
		free(lmodel); //finally free the rest of the memory allocated to the model
	} //if (lmodel!=0) {

	return(1);	
} //int FreeModel(MODEL *lmodel)

//free all the malloc'd memory for all loaded models
int FreeModelList(void)
{
	MODEL *lmodel,*tmodel;

	lmodel=F3DStatus.imodel;
	while(lmodel!=0) {
		tmodel=lmodel;
		lmodel=(MODEL *)tmodel->next;
		FreeModel(tmodel);		
	} //while(lmodel!=0) {

	F3DStatus.imodel=0;

	return(1);
} //int FreeModelList(void)


//free all the malloc'd memory for all lights
int FreeLightList(void)
{
	LIGHT *llight;
	LIGHT *tlight;

	llight=F3DStatus.ilight;
	while(llight!=0) {
		tlight=llight;
		//FreeLight(llight);
		llight=(LIGHT *)llight->next;
		free(tlight);
	} //while(lmodel!=0) {

	F3DStatus.ilight=0;

	return(1);
} //int FreeLightList(void)

//free all the malloc'd memory for all cameras
int FreeCameraList(void)
{
	CAMERA *lcamera;
	CAMERA *tcamera;

	lcamera=F3DStatus.icamera;
	while(lcamera!=0) {
		//FreeCamera(lcamera);
		tcamera=lcamera;
		lcamera=(CAMERA *)lcamera->next;
		free(tcamera);
	} //while(lmodel!=0) {

	F3DStatus.icamera=0;

	return(1);
} //int FreeCameraList(void)


//only deletes the object list and doesn't free the models, lights, or camera
int FreeObjectList(OBJLIST **lobjlist)
{
	OBJLIST *tobjlist;
	
	while(*lobjlist!=0) {
		tobjlist=*lobjlist;
		*lobjlist=(OBJLIST *)(*lobjlist)->next;
		free(tobjlist);		
	} //while(lobjlist!=0) {

	*lobjlist=0;

	return(1);
} //int FreeObjectList(void)


//return a pointer to a model
MODEL* GetModel(char *name)
{
MODEL *lmodel;

lmodel=F3DStatus.imodel;
while (lmodel!=0) {
	if (!(strcmp(lmodel->name,name))) {
		return(lmodel);
	}
  lmodel=(MODEL *)lmodel->next;
}  //while lmodel!=0

return(0);
}  //GetModel


//return a pointer to a camera
CAMERA* GetCameraByName(char *name)
{
CAMERA *lcamera;

lcamera=F3DStatus.icamera;
while (lcamera!=0) {
	if (!(strcmp(lcamera->name,name))) {
		return(lcamera);
	}
  lcamera=(CAMERA *)lcamera->next;
}  //while lcamera!=0

return(0);
}  //GetCameraByName


//return a pointer to a camera
CAMERA* GetCameraByNum(int num)
{
CAMERA *lcamera;
int i;

lcamera=F3DStatus.icamera;
i=1; //first camera is camera 1
while (lcamera!=0) {
	if (i==num) {
		return(lcamera);
	}
	i++;
  lcamera=(CAMERA *)lcamera->next;
}  //while lcamera!=0

return(0);
}  //GetCameraByNum

//return a pointer to a light
LIGHT* GetLight(char *name)
{
LIGHT *llight;

llight=F3DStatus.ilight;
while (llight!=0) {
	if (!(strcmp(llight->name,name))) return(llight);
  llight=(LIGHT *)llight->next;
}  //while llight!=0

return(0);
}  //GetLight




//presumes lmodel has been initially memset to 0, and has name and scale set
//currently presumes clockwise polygons, and traditional axis with positive x=right,y=up,z=forward
int MakeModel_Polyhedron6Sides(MODEL *lmodel)
{
	int i,j;
	POINT3D scale;
	//POINT3D angle;

	scale.x=lmodel->scale.x;
	scale.y=lmodel->scale.y;
	scale.z=lmodel->scale.z;

	//add points
	lmodel->numpoint=8;
	lmodel->point=(POINT3D *)malloc(sizeof(POINT3D)*8);
	lmodel->point[0].x=-scale.x/2.0;
	lmodel->point[0].y=scale.y/2.0;
	lmodel->point[0].z=-scale.z/2.0;
	lmodel->point[1].x=scale.x/2.0;
	lmodel->point[1].y=scale.y/2.0;
	lmodel->point[1].z=-scale.z/2.0;
	lmodel->point[2].x=scale.x/2.0;
	lmodel->point[2].y=-scale.y/2.0;
	lmodel->point[2].z=-scale.z/2.0;
	lmodel->point[3].x=-scale.x/2.0;
	lmodel->point[3].y=-scale.y/2.0;
	lmodel->point[3].z=-scale.z/2.0;
	lmodel->point[4].x=-scale.x/2.0;
	lmodel->point[4].y=scale.y/2.0;
	lmodel->point[4].z=scale.z/2.0;
	lmodel->point[5].x=scale.x/2.0;
	lmodel->point[5].y=scale.y/2.0;
	lmodel->point[5].z=scale.z/2.0;
	lmodel->point[6].x=scale.x/2.0;
	lmodel->point[6].y=-scale.y/2.0;
	lmodel->point[6].z=scale.z/2.0;
	lmodel->point[7].x=-scale.x/2.0;
	lmodel->point[7].y=-scale.y/2.0;
	lmodel->point[7].z=scale.z/2.0;

	//allocate memory for the apoints - which hold the points after they have been transformed by the model axes
	lmodel->apoint=(POINT3D *)malloc(sizeof(POINT3D)*8);

	//allocate memory for the cpoints - which hold the points after they have been clipped to a 3D volume
	lmodel->cpoint=(CLIPPOINT3D *)malloc(sizeof(CLIPPOINT3D)*8);

	//allocate memory for the axis associated with each point
	lmodel->pointaxis=(u32 *)malloc(sizeof(u32)*8);
	for(i=0;i<lmodel->numpoint;i++) {
		lmodel->pointaxis[i]=0;
	} //for(i=0;i<lmodel-numpoint;i++) {
	//transform points to DEFAULT_TRIORDINATE_SYSTEM if not standard +x=right,+y=up,+z=forward view
	//if (F3DStatus.flags&F3D_USE_CNC_ORIENTATION) {
		//set rotation: 90 degrees around the traditional x axis, 180 degrees around the traditional z axis
		//in radians
	//	angle.x=PI/2;
	//	angle.y=PI;
	//	angle.z=0;
	//	RotatePoints(lmodel->numpoint,lmodel->point,&angle);
	//}  //if (F3DStatus.flags&F3D_USE_CNC_ORIENTATION) {


	//6 polygons
	lmodel->numpolygon=6;
	lmodel->polygon=(POLYGON *)malloc(sizeof(POLYGON)*lmodel->numpolygon);
	lmodel->polygon[0].p[0]=0;
	lmodel->polygon[0].p[1]=1;
	lmodel->polygon[0].p[2]=2;
	lmodel->polygon[0].p[3]=3;
	lmodel->polygon[1].p[0]=1;
	lmodel->polygon[1].p[1]=5;
	lmodel->polygon[1].p[2]=6;
	lmodel->polygon[1].p[3]=2;
	lmodel->polygon[2].p[0]=5;
	lmodel->polygon[2].p[1]=4;
	lmodel->polygon[2].p[2]=7;
	lmodel->polygon[2].p[3]=6;
	lmodel->polygon[3].p[0]=4;
	lmodel->polygon[3].p[1]=0;
	lmodel->polygon[3].p[2]=3;
	lmodel->polygon[3].p[3]=7;
	lmodel->polygon[4].p[0]=4;
	lmodel->polygon[4].p[1]=5;
	lmodel->polygon[4].p[2]=1;
	lmodel->polygon[4].p[3]=0;
	lmodel->polygon[5].p[0]=3;
	lmodel->polygon[5].p[1]=2;
	lmodel->polygon[5].p[2]=6;
	lmodel->polygon[5].p[3]=7;

	for(i=0;i<lmodel->numpolygon;i++)  { 
		lmodel->polygon[i].numpoint=4;
		lmodel->polygon[i].axis=0;
		lmodel->polygon[i].transparency=0.0;  
		lmodel->polygon[i].color[0]=0xffffff;
		lmodel->polygon[i].color[1]=0xffffff;
		lmodel->polygon[i].color[2]=0xffffff;
		lmodel->polygon[i].color[3]=0xffffff;
		lmodel->polygon[i].u[0]=0.0;
		lmodel->polygon[i].u[1]=1.0;
		lmodel->polygon[i].u[2]=1.0;
		lmodel->polygon[i].u[3]=0.0;
		lmodel->polygon[i].v[0]=0.0;
		lmodel->polygon[i].v[1]=0.0;
		lmodel->polygon[i].v[2]=1.0;
		lmodel->polygon[i].v[3]=1.0;
		lmodel->polygon[i].ti=0;

		//calculate polygon normal
		CalculatePolygonNormal(&lmodel->polygon[i],lmodel->point);
	}


	//don't bother with point normals, because for shapes like a 6-sided Polyhedron
	//the renderer needs to use the polygon normal
	//otherwise the normal of a point would be wrong (90 degrees off) for one polygon it is shared by
	//set the F3D_POLYGON_USE_POLYGON_NORMAL flag
	lmodel->polygon->flags|=F3D_POLYGON_USE_POLYGON_NORMAL;

	//1 axis
	lmodel->numaxis=1;
	lmodel->axis = (MODELAXIS3D *)malloc(sizeof(MODELAXIS3D)*lmodel->numaxis);
	memset(lmodel->axis,0,sizeof(MODELAXIS3D));
	lmodel->axis->m.x[0]=1.0;
	lmodel->axis->m.y[1]=1.0;
	lmodel->axis->m.z[2]=1.0;
	lmodel->axisloc = (POINT3D *)malloc(sizeof(POINT3D)*lmodel->numaxis);
	memset(lmodel->axisloc,0,sizeof(POINT3D));

	for(j=0;j<MAXSUPERAXES;j++) {
		lmodel->axis->superaxis[j]=-1;
	}

	//no textures
	lmodel->numtexture=0;

	//no point normals
	lmodel->numnormal=0;

	return(1);
} //int MakeModel_Polyhedron6Sides(MODEL *lmodel)


//presumes lmodel has been initially memset to 0, and has name and scale set
//makes 3 lines with letters X, Y, Z in respective dimensions
int MakeModel_XYZ_Axis(MODEL *lmodel)
{
	int i,j;
	POINT3D scale;
//	POINT3D angle;

	scale.x=lmodel->scale.x;
	scale.y=lmodel->scale.y;
	scale.z=lmodel->scale.z;

	//add points
	lmodel->numpoint=16;
	lmodel->point=(POINT3D *)malloc(sizeof(POINT3D)*lmodel->numpoint);
	lmodel->point[0].x=0;
	lmodel->point[0].y=0;
	lmodel->point[0].z=0;
	lmodel->point[1].x=scale.x;
	lmodel->point[1].y=0;
	lmodel->point[1].z=0;
	lmodel->point[2].x=0;
	lmodel->point[2].y=scale.y;
	lmodel->point[2].z=0;
	lmodel->point[3].x=0;
	lmodel->point[3].y=0;
	lmodel->point[3].z=scale.z;
	//X
	lmodel->point[4].x=scale.x+scale.x/10.0;
	lmodel->point[4].y=scale.y/10.0;
	lmodel->point[4].z=0;
	lmodel->point[5].x=scale.x+3*scale.x/10.0;
	lmodel->point[5].y=-scale.y/10.0;
	lmodel->point[5].z=0;
	lmodel->point[6].x=scale.x+3*scale.x/10.0;
	lmodel->point[6].y=scale.y/10.0;
	lmodel->point[6].z=0;
	lmodel->point[7].x=scale.x+scale.x/10.0;
	lmodel->point[7].y=-scale.y/10.0;
	lmodel->point[7].z=0;
	//Y
	lmodel->point[8].x=-scale.x/10.0;
	lmodel->point[8].y=scale.y+3*scale.y/10.0;
	lmodel->point[8].z=0;
	lmodel->point[9].x=0;
	lmodel->point[9].y=scale.y+2*scale.y/10.0;
	lmodel->point[9].z=0;
	lmodel->point[10].x=scale.x/10.0;
	lmodel->point[10].y=scale.y+3.0*scale.y/10.0;
	lmodel->point[10].z=0;
	lmodel->point[11].x=0;
	lmodel->point[11].y=scale.y+scale.y/10.0;
	lmodel->point[11].z=0;
	//Z
	lmodel->point[12].x=0;
	lmodel->point[12].y=scale.y/10.0;
	lmodel->point[12].z=scale.z+scale.z/10.0;
	lmodel->point[13].x=0;
	lmodel->point[13].y=scale.y/10.0;
	lmodel->point[13].z=scale.z+3*scale.z/10.0;
	lmodel->point[14].x=0;
	lmodel->point[14].y=-scale.y/10.0;
	lmodel->point[14].z=scale.z+scale.z/10.0;
	lmodel->point[15].x=0;
	lmodel->point[15].y=-scale.y/10.0;
	lmodel->point[15].z=scale.z+3*scale.z/10.0;

	//allocate memory for the apoints - which hold the points after they have been transformed by the model axes
	lmodel->apoint=(POINT3D *)malloc(sizeof(POINT3D)*lmodel->numpoint);

	//allocate memory for the cpoints - which hold the points after they have been clipped to a 3D volume
	lmodel->cpoint=(CLIPPOINT3D *)malloc(sizeof(CLIPPOINT3D)*lmodel->numpoint);

	//allocate memory for the axis associated with each point
	lmodel->pointaxis=(u32 *)malloc(sizeof(u32)*lmodel->numpoint);
	for(i=0;i<lmodel->numpoint;i++) {
		lmodel->pointaxis[i]=0;
	} //for(i=0;i<lmodel-numpoint;i++) {

	//11 lines
	lmodel->numline=11;
	lmodel->line=(F3D_LINE *)malloc(sizeof(F3D_LINE)*lmodel->numline);
	lmodel->line[0].p[0]=0;
	lmodel->line[0].p[1]=1;
	lmodel->line[1].p[0]=0;
	lmodel->line[1].p[1]=2;
	lmodel->line[2].p[0]=0;
	lmodel->line[2].p[1]=3;
	lmodel->line[3].p[0]=4; //X
	lmodel->line[3].p[1]=5;
	lmodel->line[4].p[0]=7;
	lmodel->line[4].p[1]=6;
	lmodel->line[5].p[0]=8; //Y
	lmodel->line[5].p[1]=9;
	lmodel->line[6].p[0]=9;
	lmodel->line[6].p[1]=10;
	lmodel->line[7].p[0]=9;
	lmodel->line[7].p[1]=11;
	lmodel->line[8].p[0]=12; //Z
	lmodel->line[8].p[1]=13;
	lmodel->line[9].p[0]=13;
	lmodel->line[9].p[1]=14;
	lmodel->line[10].p[0]=14;
	lmodel->line[10].p[1]=15;

	for(i=0;i<lmodel->numline;i++)  { 
		if (i<3) {
			lmodel->line[i].color[0]=0xffffff;
			lmodel->line[i].color[1]=0xffffff;
		} else {
			if (i<5) {  //X
				lmodel->line[i].color[0]=0xff0000;
				lmodel->line[i].color[1]=0xff0000;
			} else {
				if (i<8) {  //Y
					lmodel->line[i].color[0]=0x00ff00;
					lmodel->line[i].color[1]=0x00ff00;
				} else {  //Z
					lmodel->line[i].color[0]=0x0000ff;
					lmodel->line[i].color[1]=0x0000ff;
				}
			}
		}
	}  //for i


	//1 axis
	lmodel->numaxis=1;
	lmodel->axis = (MODELAXIS3D *)malloc(sizeof(MODELAXIS3D)*lmodel->numaxis);
	memset(lmodel->axis,0,sizeof(MODELAXIS3D));
	lmodel->axis->m.x[0]=1.0;
	lmodel->axis->m.y[1]=1.0;
	lmodel->axis->m.z[2]=1.0;
	lmodel->axisloc = (POINT3D *)malloc(sizeof(POINT3D)*lmodel->numaxis);
	memset(lmodel->axisloc,0,sizeof(POINT3D));

	for(j=0;j<MAXSUPERAXES;j++) {
		lmodel->axis->superaxis[j]=-1;
	}

	//no textures or normals

	return(1);
} //int MakeModel_XYZ_Axis(MODEL *lmodel)

//makes a sphere or Ellipsoid around the Y axis
int MakeModelEllipsoid(POINT3D scale,int numrows,int numcolumns,MODEL *lmodel,float transparency)
{

    float theta, phi;    /* Angles used to sweep around sphere */
    float dtheta, dphi;  /* Angle between each section and ring */
    float x, y, z, v, rsintheta; /* Temporary variables */
    int i, j, k, n, m;      /* counters */
    int num_v, num_poly;  /* Internal vertex and poly count */
  	float *tu,*tv;
	int temp;
	float tempf,distance;
	float sxsy,szsy; //scalex/scaley and scalez/scaley
	POINT3D *tpnt,*tnml;
	POLYGON *tpoly;
	float cosphi,sinphi;

	//make sure parameters are valid
    if ((numrows < 1) || (numcolumns < 3) ||
        (scale.x <= 0) || (scale.y <= 0) || (scale.z <= 0))
        return 0;
    //determine number of vertices and polygons 
	num_v = numrows*numcolumns + 2;
	num_poly = (numrows)*(numcolumns-1)+(2*numcolumns);
	
	lmodel->numpoint=num_v;
	lmodel->point=(POINT3D *)malloc(sizeof(POINT3D)*num_v);
	tpnt=lmodel->point;
	memset(tpnt,0,sizeof(POINT3D)*num_v);
	lmodel->numpolygon=num_poly;
	lmodel->polygon=(POLYGON *)malloc(sizeof(POLYGON)*num_poly);
	tpoly=lmodel->polygon;
	memset(tpoly,0,sizeof(POLYGON)*num_poly);
	
	//allocate memory for the points transformed by model axes and camera
	lmodel->apoint=(POINT3D *)malloc(sizeof(POINT3D)*num_v);
	lmodel->cpoint=(CLIPPOINT3D *)malloc(sizeof(CLIPPOINT3D)*num_v);
	lmodel->numnormal=num_v;
	//allocate memory for the point surface normals, and normals after transformed by the model axes
	lmodel->normal=(POINT3D *)malloc(sizeof(POINT3D)*num_v);
	tnml=lmodel->normal;
	lmodel->anormal=(POINT3D *)malloc(sizeof(POINT3D)*num_v);

	//allocate memory for the axis associated with each point
	lmodel->pointaxis=(u32 *)malloc(sizeof(u32)*lmodel->numpoint);
	for(i=0;i<lmodel->numpoint;i++) {
		lmodel->pointaxis[i]=0;
	} //for(i=0;i<lmodel-numpoint;i++) {

	//1 axis
	lmodel->numaxis=1;
	lmodel->axis = (MODELAXIS3D *)malloc(sizeof(MODELAXIS3D)*lmodel->numaxis);
	memset(lmodel->axis,0,sizeof(MODELAXIS3D));
	lmodel->axis->m.x[0]=1.0;
	lmodel->axis->m.y[1]=1.0;
	lmodel->axis->m.z[2]=1.0;
	lmodel->axisloc = (POINT3D *)malloc(sizeof(POINT3D)*lmodel->numaxis);
	memset(lmodel->axisloc,0,sizeof(POINT3D));

	for(j=0;j<MAXSUPERAXES;j++) {
		lmodel->axis->superaxis[j]=-1;
	}


	//allocate memory for temporary texture map coordinates
	tu = (float *) malloc(sizeof(float) * num_v);
	tv = (float *) malloc(sizeof(float) * num_v);

    //make top and bottom vertices
    tpnt[0].x = 0.0;
	tpnt[0].y = scale.y;
    tpnt[0].z = 0.0;
	tu[0] = 0.0;
    tv[0] = 0.0;
	tnml[0].x=0.0;
	tnml[0].y=1.0;
	tnml[0].z=0.0;
    tpnt[num_v - 1].x = 0.0;
	tpnt[num_v - 1].y = -scale.y;
    tpnt[num_v - 1].z = 0.0;
	tu[num_v - 1] = 0.0;
    tv[num_v - 1] = 1.0;
	tnml[num_v - 1].x=0.0;
	tnml[num_v - 1].y=-1.0;
	tnml[num_v - 1].z=0.0;

    //make vertex points for rows (rings)
    dtheta = (float)(PI / (double)(numrows + 2));  //change of angle for rows
    dphi = (float)(2.0 * PI / (double) numcolumns);  //change of angle for columns
    n = 1; /* vertex being generated, begins at 1 to skip top point */
    theta = dtheta; //first increment
	sxsy=scale.x/scale.y; //how much larger or smaller x dimension is to y
	szsy=scale.z/scale.y; //how much larger or smaller z dimension is to y
    for (i = 0; i < numrows; i++) {
		y = scale.y*(float)cos(theta); /* y is the same for each ring */
        v = theta / (float)PI;     /* v is the same for each ring */
		//sintheta=(float)sin(theta);
        //rsintheta = scale.y*sintheta;  
		rsintheta = scale.y*(float)sin(theta);  
        //phi = (float)0.0;
        for (j = 0; j < numcolumns; j++) {
			phi = ((float)j/(float)numcolumns)*(2.0 * PI);

			cosphi=(float)cos(phi);
			sinphi=(float)sin(phi);
            x = -rsintheta * cosphi;
            z = -rsintheta * sinphi;
            tpnt[n].x = sxsy * x;
            tpnt[n].z = szsy * z;
            tpnt[n].y = y;
			distance=sqrt((cosphi*cosphi)+(sinphi*sinphi)+y*y);
            tnml[n].x = -cosphi/distance;
            tnml[n].z = -sinphi/distance;
            tnml[n].y = y/distance;


			tv[n] = v;
            //tu[n] = (float)(1.0 - phi / (2.0 * PI));
			tu[n] = (float)(phi / (2.0 * PI));
            n++;
        }
        theta += dtheta;
    }

    //make triangle for top and bottom caps

    for (i = 0; i < numcolumns; i++) {

//top triangle
        tpoly[i].p[0] = 0;
        tpoly[i].p[1] = 1 + ((i + 1) % numcolumns);
		tpoly[i].p[2] = i + 1;
		tpoly[i].p[3] = 0;  //not used
	
		tpoly[i].v[0]=tv[0];

		tpoly[i].u[2] = tu[i+1];
		tpoly[i].v[2] = tv[i+1];	

		if (((i + 1) % numcolumns)==0) {
			tpoly[i].u[1] = 1.0;
			tpoly[i].v[1] = tv[1];
		} else {
			tpoly[i].u[1] = tu[i+2];
			tpoly[i].v[1] = tv[i+1];	
		}  //if (((i + 1) % numcolumns)==0) {

		tpoly[i].u[0] = tpoly[i].u[2]+(tpoly[i].u[1]-tpoly[i].u[2])/2.0;

		tpoly[i].u[3] = 0;  //not used
		tpoly[i].v[3] = 0;  //not used

		tpoly[i].numpoint=3;
		tpoly[i].color[0]=0x00ffffff;
		tpoly[i].color[1]=0x00ffffff;
		tpoly[i].color[2]=0x00ffffff;
		tpoly[i].color[3]=0x00ffffff;
		tpoly[i].ti=0;
		tpoly[i].transparency=transparency;
		CalculatePolygonNormal(&tpoly[i],lmodel->point);
            
	//bottom triangle
		j=num_poly-numcolumns+i;
		k=num_v-numcolumns+i-1;

		tpoly[j].p[0] = num_v - 1;
		if (k==num_v-2) {
		  tpoly[j].p[2]=k-numcolumns+1;
		} else {
		  tpoly[j].p[2]=k+1;
		}
		tpoly[j].p[1] = k;

		tpoly[j].p[3] = 0; //not used

		tpoly[j].v[0] = tv[num_v-1];

		if (i==numcolumns-1) {
			tpoly[j].u[2] =1.0;
		} else {
			tpoly[j].u[2] =tu[k+1];
		}	

		tpoly[j].v[2] = tv[k];
		tpoly[j].u[1] = tu[k];
		tpoly[j].v[1] = tv[k];

		tpoly[j].u[3] = 0.0; //not used
		tpoly[j].v[3] = 0.0; //not used

		tpoly[j].u[0]=tpoly[j].u[2]+(tpoly[j].u[1]-tpoly[j].u[2])/2.0;

		tpoly[j].numpoint=3;
		tpoly[j].color[0]=0x00ffffff;
		tpoly[j].color[1]=0x00ffffff;
		tpoly[j].color[2]=0x00ffffff;
		tpoly[j].color[3]=0x00ffffff;
		tpoly[j].ti=0;
        tpoly[j].transparency=transparency;    

		CalculatePolygonNormal(&tpoly[j],lmodel->point);
	} //for i

    //make quads for the rings
    m = 1; /* first vertex in current ring,begins at 1 to skip top point*/
    n = numcolumns; //number of quads being made
	for (i = 0; i < numrows-1; i++) {
        for (j = 0; j < numcolumns; j++) {
            tpoly[n].p[0] = m + j;
            tpoly[n].p[1] = m + numcolumns + j;
            tpoly[n].p[2] = m + numcolumns + ((j + 1) % numcolumns);
			tpoly[n].p[3] = m + ((j + 1) % numcolumns);

			tpoly[n].u[0] = tu[m + j];
			tpoly[n].v[0] = tv[m + j];
	        tpoly[n].u[1] = tu[m + numcolumns + j];
			tpoly[n].v[1] = tv[m + numcolumns + j];

			if (tu[m + numcolumns + ((j + 1) % numcolumns)]<tu[m + numcolumns + j]) { //last one
			  tpoly[n].u[2] = 1.0;
			  tpoly[n].u[3] = 1.0;
			} else {
		      tpoly[n].u[2] = tu[m + numcolumns + ((j + 1) % numcolumns)];
			  tpoly[n].u[3] = tu[m + ((j + 1) % numcolumns)];
			}
			tpoly[n].v[2] = tv[m + numcolumns + ((j + 1) % numcolumns)];
			tpoly[n].v[3] = tv[m + ((j + 1) % numcolumns)];

			tpoly[n].numpoint=4;
			tpoly[n].color[0]=0x00ffffff;
			tpoly[n].color[1]=0x00ffffff;
			tpoly[n].color[2]=0x00ffffff;
			tpoly[n].color[3]=0x00ffffff;
			tpoly[n].ti=0;
			tpoly[n].transparency=transparency;
			
        
//#if CLOCKWISE_POLYGONS
			temp=tpoly[n].p[1];
			tpoly[n].p[1]=tpoly[n].p[3];
			tpoly[n].p[3]=temp;
			tempf=tpoly[n].u[1];
			tpoly[n].u[1]=tpoly[n].u[3];
			tpoly[n].u[3]=tempf;
			tempf=tpoly[n].v[1];
			tpoly[n].v[1]=tpoly[n].v[3];
			tpoly[n].v[3]=tempf;
//#endif
			CalculatePolygonNormal(&tpoly[n],lmodel->point);
		n++;
        } //        for (j = 0; j < numcolumns; j++) {
        m += numcolumns;
    } //for (i = 0; i < numrows-1; i++) {

	free(tu);  
	free(tv);

    return(1);
} //int MakeModelEllipsoid(POINT3D scale,int numrows,int numcolumns,MODEL *lmodel,float transparency)



//make a model cylinder
int MakeModelCylinder(POINT3D scale,int numcolumns,MODEL *lmodel,float transparency,unsigned int flags)
{
int num_v,num_poly;
float *tu,*tv;
float dphi,phi;
POINT3D *tpnt,*tnml;
POLYGON *tpoly;
int i,j,m,n,MakeTop,MakeBottom;
float x,y,z;
float cosphi,sinphi;
	
	MakeTop=0;
	MakeBottom=0;
	if (flags&F3D_MAKE_NEW_CYLINDER_MAKE_TOP) {
		MakeTop=1;
	}
	if (flags&F3D_MAKE_NEW_CYLINDER_MAKE_BOTTOM) {
		MakeBottom=1;
	}
	num_v = numcolumns*2+2; //2 center points for top and bottom cicles
	num_poly = numcolumns*(1+MakeTop+MakeBottom);	  

	lmodel->numpoint=num_v;
	lmodel->point=(POINT3D *)malloc(sizeof(POINT3D)*num_v);
	tpnt=lmodel->point;
	memset(tpnt,0,sizeof(POINT3D)*num_v);
	lmodel->numpolygon=num_poly;
	lmodel->polygon=(POLYGON *)malloc(sizeof(POLYGON)*num_poly);
	tpoly=lmodel->polygon;
	memset(tpoly,0,sizeof(POLYGON)*num_poly);
	
	//allocate memory for the points transformed by model axes and camera
	lmodel->apoint=(POINT3D *)malloc(sizeof(POINT3D)*num_v);
	lmodel->cpoint=(CLIPPOINT3D *)malloc(sizeof(CLIPPOINT3D)*num_v);
	lmodel->numnormal=num_v;
	//allocate memory for the point surface normals, and normals after transformed by the model axes
	lmodel->normal=(POINT3D *)malloc(sizeof(POINT3D)*num_v);
	tnml=lmodel->normal;
	lmodel->anormal=(POINT3D *)malloc(sizeof(POINT3D)*num_v);

	//allocate memory for the axis associated with each point
	lmodel->pointaxis=(u32 *)malloc(sizeof(u32)*lmodel->numpoint);
	memset(lmodel->pointaxis,0,sizeof(u32)*lmodel->numpoint);

	//1 axis
	lmodel->numaxis=1;
	lmodel->axis = (MODELAXIS3D *)malloc(sizeof(MODELAXIS3D)*lmodel->numaxis);
	memset(lmodel->axis,0,sizeof(MODELAXIS3D));
	lmodel->axis->m.x[0]=1.0;
	lmodel->axis->m.y[1]=1.0;
	lmodel->axis->m.z[2]=1.0;
	lmodel->axisloc = (POINT3D *)malloc(sizeof(POINT3D)*lmodel->numaxis);
	memset(lmodel->axisloc,0,sizeof(POINT3D));

	for(j=0;j<MAXSUPERAXES;j++) {
		lmodel->axis->superaxis[j]=-1;
	}

	//allocate memory for temporary texture map coordinates
	tu = (float *) malloc(sizeof(float) * num_v);
	tv = (float *) malloc(sizeof(float) * num_v);

//Make vertex points for top and bottom rings	
	y = (float)scale.y;


	if (MakeTop) {
		//if a top and bottom there is a point in the top and bottom center
		//top and bottom circle center
		tpnt[0].x=0.0; 
		tpnt[0].y=y; 
		tpnt[0].z=0.0;  
		tu[0] = 0.5;
		tv[0] = 0.5;
	} //	if (flags&F3D_MAKE_NEW_CYLINDER_MAKE_TOP) {


	if (MakeBottom) {
		tpnt[numcolumns+1].x=0.0; 
		tpnt[numcolumns+1].y=-y; 
		tpnt[numcolumns+1].z=0.0;  
		tu[numcolumns+1] = 0.5;
		tv[numcolumns+1] = 0.5;
	} //if (flags&F3D_MAKE_NEW_CYLINDER_MAKE_BOTTOM) {

	dphi =(float)(2.0*PI) /(float)numcolumns;
   
    for (i = MakeTop;i<numcolumns+MakeTop;i++) {
		//vertices of top ring
    	phi = ((float)(i-MakeTop)/(float)numcolumns)*(2.0 * PI);
		cosphi=cos(phi);
		sinphi=sin(phi);
		x = -scale.x*cosphi;
		z = scale.z*sinphi;
        tpnt[i].x = x;
        tpnt[i].y = y;
        tpnt[i].z = z;
        tnml[i].x = -cosphi;//0;
        tnml[i].y = 0.0;//1.0;
        tnml[i].z = sinphi;//0;

		//calculate tu,tv for top and bottom circle to use later
        tu[i] = (scale.x+x)/(scale.x*2.0);
		tv[i] = (scale.z-z)/(scale.z*2.0);

		j=i+numcolumns;
		//vertices of bottom ring
		tpnt[j].x = x;
        tpnt[j].y = -y;
        tpnt[j].z = z;
        tnml[j].x = -cosphi;//x/distance;//0;
        tnml[j].y = 0.0;//-1.0;
        tnml[j].z = sinphi;//0;
			
        tu[j] = tu[i];//(scale.x+x)/(scale.x*2.0);
		tv[j] = 1.0;//(scale.z+-z)/(scale.z*2.0);
	
    } //i


	if (MakeTop || MakeBottom) {		
		//make triangles for top and bottom 

		//top and bottom will have F3D_POLYGON_USE_POLYGON_NORMAL flag and the side can have Gouraud shading
		for (i = 0; i < numcolumns; i++) {

			//presumes clockwise	
			if (MakeTop) {
				tpoly[i].flags=F3D_POLYGON_USE_POLYGON_NORMAL;  //or else Gouraud lighting won't work for these polygons because vertices has sideways normals
		        //tpoly[i].n.x=0;
				//tpoly[i].n.y=1.0;
				//tpoly[i].n.z=0;

				//top triangles
				tpoly[i].p[0] = 0;
				tpoly[i].p[1] = i+1;  //0,1,2   0,2,3, etc.
				tpoly[i].p[2] = (i==numcolumns-1)?1:i+2;

				tpoly[i].p[3] = 0;

				tpoly[i].u[0] = 0.5;
				tpoly[i].v[0] = 0.5;//tv[0];
				tpoly[i].u[1] = tu[i+1];		
				tpoly[i].v[1] = tv[i+1];
				tpoly[i].u[2] = tu[(i==numcolumns-1)?1:i+2];
				tpoly[i].v[2] = tv[(i==numcolumns-1)?1:i+2];
		

				tpoly[i].u[3] = 0.0;//not used
				tpoly[i].v[3] = 0.0;//not used
				tpoly[i].numpoint=3;
				tpoly[i].color[0]=0xffffff;
				tpoly[i].color[1]=0xffffff;
				tpoly[i].color[2]=0xffffff;
				tpoly[i].color[3]=0xffffff;
				tpoly[i].ti=0;

				CalculatePolygonNormal(&tpoly[i],lmodel->point);
			} //		if (flags&F3D_MAKE_NEW_CYLINDER_MAKE_TOP) {

			if (MakeBottom) {
				//bottom triangles
				tpoly[j].flags=F3D_POLYGON_USE_POLYGON_NORMAL;
		        //tpoly[j].n.x=0;
				//tpoly[j].n.y=-1.0;
				//tpoly[j].n.z=0;

				j=i+numcolumns*MakeTop;
				tpoly[j].p[0] = numcolumns+1;  //center
				tpoly[j].p[1] = j+2;
				tpoly[j].p[2] = j+3-(j==numcolumns*2-1)*numcolumns;  //0,1,2  0,2,3 etc.

				tpoly[j].p[3] = 0;

				tpoly[j].u[0] = 0.5;
				tpoly[j].v[0] = tv[0];
				tpoly[j].u[2] = tu[j+3];		
				tpoly[j].v[2] = tv[j+3];
				if (i==numcolumns-1) {
					tpoly[j].v[2] = tu[numcolumns*MakeTop+2];		
					tpoly[j].u[2] = tv[numcolumns*MakeTop+2];
				}
			
				tpoly[j].u[1] = tu[j+2];
				tpoly[j].v[1] = tv[j+2];

				tpoly[j].u[3] = 0.0;//not used
				tpoly[j].v[3] = 0.0;//not used
				tpoly[j].numpoint=3;
				tpoly[j].color[0]=0xffffff;
				tpoly[j].color[1]=0xffffff;
				tpoly[j].color[2]=0xffffff;
				tpoly[j].color[3]=0xffffff;
				tpoly[j].ti=2;//presumes a texture

				CalculatePolygonNormal(&tpoly[j],lmodel->point);
				} //		if (flags&F3D_MAKE_NEW_CYLINDER_MAKE_BOTTOM) {
			}  //for i
		}//	if ((flags&F3D_MAKE_NEW_CYLINDER_MAKE_TOP) || (flags&F3D_MAKE_NEW_CYLINDER_MAKE_BOTTOM)) {

		//Generate quads for the side of the cylinder
		m = MakeTop; //first vertex in current ring,begins at 1 to skip top point
		n = numcolumns*(MakeTop+MakeBottom);  //first polygon is after top and bottom circle polygons
		for (i = 0; i < numcolumns; i++) {
			tpoly[n].p[0] = m + i + 1;
			tpoly[n].p[1] = m + i;
			tpoly[n].p[2] = m + numcolumns + i;
			tpoly[n].p[3] = m + numcolumns + i+1;
			if (i==numcolumns-1) {
				tpoly[n].p[0]=MakeTop;
				tpoly[n].p[3]=numcolumns+MakeTop;
			}

			tpoly[n].u[0] = (float)(i+1)/(float)numcolumns;
			tpoly[n].v[0] = 0.0;
			tpoly[n].u[1] = (float)i/(float)numcolumns;
			tpoly[n].v[1] = 0.0;
			tpoly[n].u[2]=tpoly[n].u[1];
			tpoly[n].u[3]=tpoly[n].u[0];
			
			tpoly[n].v[2] = 1.0;
			tpoly[n].v[3] = 1.0;

			tpoly[n].numpoint=4;
			tpoly[n].color[0]=0x00ffffff;
			tpoly[n].color[1]=0x00ffffff;
			tpoly[n].color[2]=0x00ffffff;
			tpoly[n].color[3]=0x00ffffff;
			tpoly[n].ti=1;
  
			CalculatePolygonNormal(&tpoly[n],lmodel->point);

			n++;
		} //i

	free(tu);
	free(tv);
	return(1);
} //int MakeModelCylinder(POINT3D scale,int numcolumns,MODEL *lmodel,float transparency)


int SaveModel(MODEL *lmodel,char *filename) 
{
	FILE *fptr;
	char tstr[FTMedStr];
	MODELAXIS3D *t;
	POINT3D *p;
	MODELJOINT *joint;
	F3D_LINE *l;
	POLYGON *po;
	int i,j;

	//perhaps should be in utf-8

	fptr=fopen(filename,"wb");
	if (fptr!=0) {		
		sprintf(tstr,"NAME: %s\n",lmodel->name);
		fwrite(tstr,1,strlen(tstr),fptr);
		if (lmodel->flags==0) {
			sprintf(tstr,"FLAGS: NONE\n");
		} else {
			sprintf(tstr,"FLAGS: ");		
			if (lmodel->flags&MODEL_NORENDER) {
				sprintf(tstr,"%sMODEL_NORENDER|",tstr);	
			}
			if (lmodel->flags&MODEL_VIDEOLOOP) {
				sprintf(tstr,"%sMODEL_VIDEOLOOP|",tstr);	
			}
			if (lmodel->flags&MODEL_VIDEOTEXTURE) {
				sprintf(tstr,"%sMODEL_VIDEOTEXTURE|",tstr);	
			}
			if (lmodel->flags&MODEL_NOLIGHTING) {
				sprintf(tstr,"%sMODEL_NOLIGHTING|",tstr);	
			}
			tstr[strlen(tstr)-1]=0; //remove last |
			sprintf(tstr,"%s\n",tstr);	//and add carriage return
		}//if (lmodel->flags==0) {
		fwrite(tstr,1,strlen(tstr),fptr);

		sprintf(tstr,"SCALE: %0.4f,%0.4f,%0.4f\n",lmodel->scale.x,lmodel->scale.y,lmodel->scale.z);
		fwrite(tstr,1,strlen(tstr),fptr);
		sprintf(tstr,"LOCATION: %0.4f,%0.4f,%0.4f\n",lmodel->location.x,lmodel->location.y,lmodel->location.z);
		fwrite(tstr,1,strlen(tstr),fptr);
		sprintf(tstr,"SPEED: %0.4f,%0.4f,%0.4f\n",lmodel->speed.x,lmodel->speed.y,lmodel->speed.z);
		fwrite(tstr,1,strlen(tstr),fptr);
		sprintf(tstr,"ROTSPEED: %0.4f,%0.4f,%0.4f\n",lmodel->rotspeed.x,lmodel->rotspeed.y,lmodel->rotspeed.z);
		fwrite(tstr,1,strlen(tstr),fptr);
		sprintf(tstr,"INCREMENT: %0.4f,%0.4f,%0.4f\n",lmodel->increment.x,lmodel->increment.y,lmodel->increment.z);
		fwrite(tstr,1,strlen(tstr),fptr);
		sprintf(tstr,"MAXEXTENT: %0.4f,%0.4f,%0.4f\n",lmodel->MaxExtent.x,lmodel->MaxExtent.y,lmodel->MaxExtent.z);
		fwrite(tstr,1,strlen(tstr),fptr);
		sprintf(tstr,"MINEXTENT: %0.4f,%0.4f,%0.4f\n",lmodel->MinExtent.x,lmodel->MinExtent.y,lmodel->MinExtent.z);
		fwrite(tstr,1,strlen(tstr),fptr);

		sprintf(tstr,"NUMBER_OF_AXES: %d\n",lmodel->numaxis);
		fwrite(tstr,1,strlen(tstr),fptr);
		sprintf(tstr, "NUMBER_OF_JOINTS: %d\n", lmodel->numjoint);
		fwrite(tstr, 1, strlen(tstr), fptr);
		sprintf(tstr,"NUMBER_OF_POINTS: %d\n",lmodel->numpoint);
		fwrite(tstr,1,strlen(tstr),fptr);
		sprintf(tstr,"NUMBER_OF_NORMALS: %d\n",lmodel->numnormal);
		fwrite(tstr,1,strlen(tstr),fptr);
		sprintf(tstr,"NUMBER_OF_LINES: %d\n",lmodel->numline);
		fwrite(tstr,1,strlen(tstr),fptr);
		sprintf(tstr,"NUMBER_OF_POLYGONS: %d\n",lmodel->numpolygon);
		fwrite(tstr,1,strlen(tstr),fptr);
		sprintf(tstr,"NUMBER_OF_TEXTURES: %d\n",lmodel->numtexture);
		fwrite(tstr,1,strlen(tstr),fptr);

		//write AXES
		sprintf(tstr,"AXES: (%d)\n",lmodel->numaxis);
		fwrite(tstr,1,strlen(tstr),fptr);
		for(i=0;i<lmodel->numaxis;i++)  {
			t=&lmodel->axis[i];
			sprintf(tstr,"%d: %f,%f,%f,%f,%f,%f,%f,%f,%f\n",i,t->m.x[0],t->m.y[0],t->m.z[0],t->m.x[1],t->m.y[1],t->m.z[1],t->m.x[2],t->m.y[2],t->m.z[2]);
			fwrite(tstr,1,strlen(tstr),fptr);
			sprintf(tstr,"AXIS_LOCATION: %f,%f,%f\n",t->loc.x,t->loc.y,t->loc.z);
			fwrite(tstr,1,strlen(tstr),fptr);
			sprintf(tstr, "NUMBER_OF_SUPERAXES: ");
			fwrite(tstr, 1, strlen(tstr), fptr);
			sprintf(tstr, "%d\n", t->numsuperaxis);
			fwrite(tstr, 1, strlen(tstr), fptr);
			sprintf(tstr,"SUPERAXES: ");
			fwrite(tstr,1,strlen(tstr),fptr);
			if (t->numsuperaxis>0) {
				sprintf(tstr, "SUPERAXES: ");
				fwrite(tstr, 1, strlen(tstr), fptr);
				for (j = 0; j < t->numsuperaxis; j++) {
					if (j < t->numsuperaxis - 1) {
						sprintf(tstr, "%d,", t->superaxis[j]);
					}
					else {
						sprintf(tstr, "%d\n", t->superaxis[j]);
					}
					fwrite(tstr, 1, strlen(tstr), fptr);
				}  //for j
			} //if (t->numsuperaxis>0) {
			sprintf(tstr, "NUMBER_OF_JOINTPOINTS: %d\n", t->numjointpoint);
			fwrite(tstr, 1, strlen(tstr), fptr);
			for (j = 0; j < t->numjointpoint; j++) {
				sprintf(tstr, "%d: %f,%f,%f\n", j, t->jp[j].x, t->jp[j].y, t->jp[j].z);
				fwrite(tstr, 1, strlen(tstr), fptr);
			}  //for j
		} //for i

		//write joints
		sprintf(tstr, "JOINTS: (%d)\n", lmodel->numjoint);
		fwrite(tstr, 1, strlen(tstr), fptr);
		for (i = 0; i<lmodel->numjoint; i++) {
			joint = &lmodel->joint[i];
			sprintf(tstr, "%d: %d,%d,%d,%d\n", i, joint->a[0], joint->a[1], joint->p[0], joint->p[1]);
			fwrite(tstr, 1, strlen(tstr), fptr);
		}

		//write points
		sprintf(tstr,"POINTS: (%d)\n",lmodel->numpoint);
		fwrite(tstr,1,strlen(tstr),fptr);
		for(i=0;i<lmodel->numpoint;i++)  {
			p=&lmodel->point[i];
			sprintf(tstr,"%d: %f,%f,%f\n",i,p->x,p->y,p->z);
			fwrite(tstr,1,strlen(tstr),fptr);
		}

		//write normals
		sprintf(tstr,"NORMALS: (%d)\n",lmodel->numnormal);
		fwrite(tstr,1,strlen(tstr),fptr);
		for(i=0;i<lmodel->numnormal;i++)  {
			p=&lmodel->normal[i];
			sprintf(tstr,"%d: %f,%f,%f\n",i,p->x,p->y,p->z);
			fwrite(tstr,1,strlen(tstr),fptr);
		}

		//write lines
		sprintf(tstr,"LINES: (%d)\n",lmodel->numline);
		fwrite(tstr,1,strlen(tstr),fptr);
		for(i=0;i<lmodel->numline;i++)  {
			l=&lmodel->line[i];
			//sprintf(tstr,"%d: %d,COLOR:%06x,%d,COLOR:%06x\n",i,l->p[0],l->color[0],l->p[1],l->color[1]);
			//fwrite(tstr,1,strlen(tstr),fptr);
			sprintf(tstr,"LINE %d:\n",i);
			fwrite(tstr,1,strlen(tstr),fptr);
			//sprintf(tstr,"AXIS: %d\n",po->axis);
			//fwrite(tstr,1,strlen(tstr),fptr);
			sprintf(tstr,"LINE_POINTS: (2) %d,%d\n",l->p[0],l->p[1]);
			fwrite(tstr,1,strlen(tstr),fptr);
			//sprintf(tstr,"U: %f,%f,%f,%f\n",po->u[0],po->u[1],po->u[2],po->u[3]);
			//fwrite(tstr,1,strlen(tstr),fptr);
			//sprintf(tstr,"V: %f,%f,%f,%f\n",po->v[0],po->v[1],po->v[2],po->v[3]);
			//fwrite(tstr,1,strlen(tstr),fptr);
			sprintf(tstr,"LINE_COLOR: 0x%06x,0x%06x\n",l->color[0],l->color[1]);
			fwrite(tstr,1,strlen(tstr),fptr);
		}

		//write polygons
		sprintf(tstr,"POLYGONS: (%d)\n",lmodel->numpolygon);
		fwrite(tstr,1,strlen(tstr),fptr);
		for(i=0;i<lmodel->numpolygon;i++)  {
			po=&lmodel->polygon[i];
			sprintf(tstr,"POLYGON %d:\n",i);
			fwrite(tstr,1,strlen(tstr),fptr);
			sprintf(tstr,"POLYGON_FLAGS: ");
			if (lmodel->polygon[i].flags&F3D_POLYGON_USE_POLYGON_NORMAL) {
				sprintf(tstr,"%sF3D_POLYGON_USE_POLYGON_NORMAL",tstr);
			}
			sprintf(tstr,"%s\n",tstr);
			fwrite(tstr,1,strlen(tstr),fptr);
			sprintf(tstr,"POLYGON_AXIS: %d\n",po->axis);
			fwrite(tstr,1,strlen(tstr),fptr);
			if (po->numpoint==4) {
				sprintf(tstr,"POLYGON_POINTS: (%d) %d,%d,%d,%d\n",po->numpoint,po->p[0],po->p[1],po->p[2],po->p[3]);
			} else {
				sprintf(tstr,"POLYGON_POINTS: (%d) %d,%d,%d\n",po->numpoint,po->p[0],po->p[1],po->p[2]);
			}
			fwrite(tstr,1,strlen(tstr),fptr);
			if (po->numpoint==4) {
				sprintf(tstr,"POLYGON_U: %f,%f,%f,%f\n",po->u[0],po->u[1],po->u[2],po->u[3]);
			} else {
				sprintf(tstr,"POLYGON_U: %f,%f,%f\n",po->u[0],po->u[1],po->u[2]);
			}
			fwrite(tstr,1,strlen(tstr),fptr);
			if (po->numpoint==4) {
				sprintf(tstr,"POLYGON_V: %f,%f,%f,%f\n",po->v[0],po->v[1],po->v[2],po->v[3]);
			} else {
				sprintf(tstr,"POLYGON_V: %f,%f,%f\n",po->v[0],po->v[1],po->v[2]);
			}
			fwrite(tstr,1,strlen(tstr),fptr);
			if (po->numpoint==4) {
				sprintf(tstr,"POLYGON_COLOR: 0x%06x,0x%06x,0x%06x,0x%06x\n",po->color[0],po->color[1],po->color[2],po->color[3]);
			} else {
				sprintf(tstr,"POLYGON_COLOR: 0x%06x,0x%06x,0x%06x\n",po->color[0],po->color[1],po->color[2]);
			}
			fwrite(tstr,1,strlen(tstr),fptr);
			sprintf(tstr,"POLYGON_NORMAL: %f,%f,%f\n",po->n.x,po->n.y,po->n.z);
			fwrite(tstr,1,strlen(tstr),fptr);
			sprintf(tstr,"POLYGON_TEXTURE: %d\n",po->ti);
			fwrite(tstr,1,strlen(tstr),fptr);
			sprintf(tstr,"POLYGON_TRANSPARENCY: %f\n",po->transparency);
			fwrite(tstr,1,strlen(tstr),fptr);
		}

		//write texture file name
		sprintf(tstr,"TEXTURES: (%d)\n",lmodel->numtexture);
		fwrite(tstr,1,strlen(tstr),fptr);
		for(i=0;i<lmodel->numtexture;i++)  {
			sprintf(tstr,"%d: ",i);
			fwrite(tstr,1,strlen(tstr),fptr);
			sprintf(tstr,"%s\n",lmodel->texture[i].name);
			fwrite(tstr,1,strlen(tstr),fptr);
		}

		fclose(fptr);

		//update current filename of model
		strcpy(lmodel->filename,filename);
	} else {
		sprintf(tstr,"Could not open file '%s'\n",filename);
		FTMessageBox(tstr,FTMB_OK,"Error",0);
		fprintf(stderr,"Could not open file '%s'\n",filename);
		return(0);
	} //fptr!=0

	return(1);
} //int SaveModel(MODEL *lmodel,char *filename) {

MODEL* LoadModel(char *filename) 
{

	FILE *fptr;
	char tstr[FTMedStr];
	char tline[FTMedStr];
	POLYGON *po;
	int i,j,k;
	MODEL *lmodel;
	int fSize,bSize,result;
	char *buf,*param;
	char Section[NUMMODELFILESECTIONS][100];
	char PolygonSection[NUMPOLYGONSECTIONS][100];
	int match,parammatch,polymatch,CurrentPolygonSection;
	int NumLines,CurLine,CurChar,DoneReadingPolygons,tlinelen,OldFormat;
	int count;
	u32 *pa;
	MODELAXIS3D *axis;

	fptr=fopen(filename,"rb");
	if (fptr!=0) {

		//allocate space for the new model
		lmodel=(MODEL *)malloc(sizeof(MODEL));
		memset(lmodel,0,sizeof(MODEL));
		strcpy(lmodel->filename,filename);

		// determine file size:
#if Linux
		fseeko(fptr,0,SEEK_END);
		fSize=ftello(fptr);
#endif
#if WIN32
		_fseeki64(fptr,0,SEEK_END);
		fSize=_ftelli64(fptr);
#endif
		rewind(fptr);

		// allocate memory to contain the whole file:
		buf=(char *)malloc(fSize+1);//+1 for buf[fSize]=0
		if (buf == NULL) {
			sprintf(tstr,"Allocating memory for model file failed.\n");
			FTMessageBox(tstr,FTMB_OK,"Error",0);
			return(0);
		}

		//read in the entire file
		//copy the file into the buffer:
		result=fread(buf,1,fSize,fptr);
		if (result != fSize) {
			sprintf(tstr,"Error reading model file %s\n",filename);
			FTMessageBox(tstr,FTMB_OK,"Error",0);
			return(0);
		}

		fclose(fptr);
		buf[fSize]=0; //close string of buffer - so strlen will give correct size
		bSize=strlen(buf);

		//set each expected section text - todo: use an enum
		strcpy(Section[MODFILE_MODEL_NAME],"NAME:");
		strcpy(Section[MODFILE_FLAGS],"FLAGS:");
		strcpy(Section[MODFILE_LOCATION],"LOCATION:");
		strcpy(Section[MODFILE_SCALE],"SCALE:");
		strcpy(Section[MODFILE_SPEED],"SPEED:");
		strcpy(Section[MODFILE_ROTSPEED],"ROTSPEED:");
		strcpy(Section[MODFILE_INCREMENT],"INCREMENT:");
		strcpy(Section[MODFILE_MAXEXTENT],"MAXEXTENT:");
		strcpy(Section[MODFILE_MINEXTENT],"MINEXTENT:");
		strcpy(Section[MODFILE_NUMAXES],"NUMBER_OF_AXES:");
		strcpy(Section[MODFILE_NUMJOINTS],"NUMBER_OF_JOINTS:");
		strcpy(Section[MODFILE_NUMPOINTS],"NUMBER_OF_POINTS:");
		strcpy(Section[MODFILE_NUMNORMALS],"NUMBER_OF_NORMALS:");
		strcpy(Section[MODFILE_NUMLINES],"NUMBER_OF_LINES:");
		strcpy(Section[MODFILE_NUMPOLYGONS],"NUMBER_OF_POLYGONS:");
		strcpy(Section[MODFILE_NUMTEXTURES],"NUMBER_OF_TEXTURES:");
		strcpy(Section[MODFILE_AXES],"AXES:");
		strcpy(Section[MODFILE_JOINTS],"JOINTS:");
		strcpy(Section[MODFILE_POINTS],"POINTS:");
		strcpy(Section[MODFILE_NORMALS],"NORMALS:");
		strcpy(Section[MODFILE_LINES],"LINES:");
		strcpy(Section[MODFILE_POLYGONS],"POLYGONS:");
		strcpy(Section[MODFILE_TEXTURES],"TEXTURES:");

		//set each expected subsection text - todo: use an enum
		strcpy(PolygonSection[MODFILE_POLYGON_POLYGON],"POLYGON");
		strcpy(PolygonSection[MODFILE_POLYGON_FLAGS],"POLYGON_FLAGS:");
		strcpy(PolygonSection[MODFILE_POLYGON_AXIS],"POLYGON_AXIS:");
		strcpy(PolygonSection[MODFILE_POLYGON_POINTS],"POLYGON_POINTS:");
		strcpy(PolygonSection[MODFILE_POLYGON_U],"POLYGON_U:");
		strcpy(PolygonSection[MODFILE_POLYGON_V],"POLYGON_V:");
		strcpy(PolygonSection[MODFILE_POLYGON_COLOR],"POLYGON_COLOR:");
		strcpy(PolygonSection[MODFILE_POLYGON_NORMAL],"POLYGON_NORMAL:");
		strcpy(PolygonSection[MODFILE_POLYGON_TEXTURE],"POLYGON_TEXTURE:");
		strcpy(PolygonSection[MODFILE_POLYGON_TRANSPARENCY],"POLYGON_TRANSPARENCY:");
		strcpy(PolygonSection[MODFILE_POLYGON_END],"TEXTURES:"); //end of polygons


		//go through each line and each word (separated by spaces or a comma)

		//determine total number of lines in order to allocate space for a pointer to each for strtok
		NumLines=0;
		i=0;
		while(i<bSize) {
			if (buf[i]==10|| (buf[i]!=10 && i==bSize-1)) {
				NumLines++;
			}
			i++;
		}  //while(i<bSize)

		CurLine=0;
		CurChar=0;
		while(CurLine<NumLines) { 
			//get a line from the file text
			F3DGetLineFromText(tline,buf+CurChar);
			CurChar+=strlen(tline);
			CurLine++;
			//get a word on this line
			param=strtok(tline," ");
			//see if this word matches any known section
			match=0;
			i=0;
			while(match==0 && i<NUMMODELFILESECTIONS) {
				if (!_stricmp(param,Section[i])) {  
					//found a match
					match=1;
					//read in data for this section
					switch(i) {
						case MODFILE_MODEL_NAME:
							//next arg is name
							param=strtok(NULL,"\n");
							strcpy(lmodel->name,param);
							//add info fprintf
						break;
						case MODFILE_FLAGS:
							//get all the model flags
							param=strtok(NULL,"|\n");
							while(param!=0) {
								//param[strlen(param)+1]=0; //need termination for strcmp
								//for some reason needs strncmp- strcmp and above line don't work
								parammatch=0;
								if (!strncmp(param,"NONE",4)) {
									lmodel->flags=0;
									parammatch=1;
								}
								if (!strncmp(param,"MODEL_NORENDER",14)) {
									lmodel->flags|=MODEL_NORENDER;
									parammatch=1;
								}
								if (!strncmp(param,"MODEL_VIDEOTEXTURE",18)) {
									lmodel->flags|=MODEL_VIDEOTEXTURE;
									parammatch=1;
								}
								if (!strncmp(param,"MODEL_VIDEOLOOP",15)) {
									lmodel->flags|=MODEL_VIDEOLOOP;
									parammatch=1;
								}
								if (!strncmp(param,"MODEL_NOLIGHTING",16)) {
									lmodel->flags|=MODEL_NOLIGHTING;
									parammatch=1;
								}
								if (!parammatch) {
									sprintf(tstr,"Unknown model flag %s",param);
									FTMessageBox(tstr,FTMB_OK,"FREE3D - Error",0);
								}
								param=strtok(NULL,"|"); //get next param
							} //while(param!=0)																	
							//add info fprintf
						break;
						case MODFILE_LOCATION:
							param=strtok(NULL,","); 
							lmodel->location.x=atof(param);
							param=strtok(NULL,","); 
							lmodel->location.y=atof(param);
							param=strtok(NULL,",\n"); 
							lmodel->location.z=atof(param);	
						break;
						case MODFILE_SCALE:
							param=strtok(NULL,","); 
							lmodel->scale.x=atof(param);
							param=strtok(NULL,","); 
							lmodel->scale.y=atof(param);
							param=strtok(NULL,",\n"); 
							lmodel->scale.z=atof(param);	
						break;
						case MODFILE_SPEED:
							param=strtok(NULL,","); 
							lmodel->speed.x=atof(param);
							param=strtok(NULL,","); 
							lmodel->speed.y=atof(param);
							param=strtok(NULL,",\n"); 
							lmodel->speed.z=atof(param);
						break;
						case MODFILE_ROTSPEED:
							param=strtok(NULL,","); 
							lmodel->rotspeed.x=atof(param);
							param=strtok(NULL,","); 
							lmodel->rotspeed.y=atof(param);
							param=strtok(NULL,",\n"); 
							lmodel->rotspeed.z=atof(param);
						break;
						case MODFILE_INCREMENT:
							param=strtok(NULL,","); 
							lmodel->increment.x=atof(param);
							param=strtok(NULL,","); 
							lmodel->increment.y=atof(param);
							param=strtok(NULL,",\n"); 
							lmodel->increment.z=atof(param);
						break;
						case MODFILE_MAXEXTENT:
							param=strtok(NULL,","); 
							lmodel->MaxExtent.x=atof(param);
							param=strtok(NULL,","); 
							lmodel->MaxExtent.y=atof(param);
							param=strtok(NULL,",\n"); 
							lmodel->MaxExtent.z=atof(param);
						break;
						case MODFILE_MINEXTENT:
							param=strtok(NULL,","); 
							lmodel->MinExtent.x=atof(param);
							param=strtok(NULL,","); 
							lmodel->MinExtent.y=atof(param);
							param=strtok(NULL,",\n"); 
							lmodel->MinExtent.z=atof(param);
						break;
						case MODFILE_NUMAXES:
							//next arg is Num of axes
							param=strtok(NULL,"\n");
							lmodel->numaxis=atoi(param);							
							if (lmodel->numaxis>0) {
								//allocate space for each axis
								lmodel->axis=(MODELAXIS3D *)malloc(sizeof(MODELAXIS3D)*lmodel->numaxis);
							}
							//add info fprintf
						break;
						case MODFILE_NUMJOINTS:
							//next arg is Num of joints
							param=strtok(NULL,"\n");
							lmodel->numjoint=atoi(param);							
							if (lmodel->numjoint>0) {
								//allocate space for each joint
								lmodel->joint=(MODELJOINT *)malloc(sizeof(MODELJOINT)*lmodel->numjoint);
							}
							//add info fprintf
						break;
						case MODFILE_NUMPOINTS:
							//next arg is Num of points
							param=strtok(NULL,"\n");
							lmodel->numpoint=atoi(param);
							if (lmodel->numpoint>0) {
								//allocate space for each point
								lmodel->point=(POINT3D *)malloc(sizeof(POINT3D)*lmodel->numpoint);
								//allocate memory for the apoints - which hold the points after they have been transformed by the model axes
								lmodel->apoint=(POINT3D *)malloc(sizeof(POINT3D)*lmodel->numpoint);
								//allocate memory for the cpoints - which hold the points after they have been clipped to a 3D volume
								lmodel->cpoint=(CLIPPOINT3D *)malloc(sizeof(CLIPPOINT3D)*lmodel->numpoint);
								//allocate memory for the axis associated with each point
								lmodel->pointaxis=(u32 *)malloc(sizeof(u32)*lmodel->numpoint);
								memset(lmodel->pointaxis,0,sizeof(u32)*lmodel->numpoint);
							}
							//add info fprintf
						break;
						case MODFILE_NUMNORMALS:  //number of normals for each point in the model
							param=strtok(NULL,"\n");
							lmodel->numnormal=atoi(param);
							//if there are no point normals they should e calculated by averaging the 
							//surface normals of all polygons the point belongs to
							if (lmodel->numnormal>0) {
								//allocate space for the surface normal of each point
								lmodel->normal=(POINT3D *)malloc(sizeof(POINT3D)*lmodel->numnormal);
								//allocate space for the surface normal of each point after axis transform
								lmodel->anormal=(POINT3D *)malloc(sizeof(POINT3D)*lmodel->numnormal);
							}
						break;
						case MODFILE_NUMLINES:
							//next arg is Num of lines
							param=strtok(NULL,"\n");
							lmodel->numline=atoi(param);
							if (lmodel->numline>0) {
								//allocate space for each line
								lmodel->line=(F3D_LINE *)malloc(sizeof(F3D_LINE)*lmodel->numline);
							}
							//add info fprintf
						break;
						case MODFILE_NUMPOLYGONS:
							//next arg is Num of polygons
							param=strtok(NULL,"\n");
							lmodel->numpolygon=atoi(param);
							if (lmodel->numpolygon>0) {
								//allocate space for each polygon
								lmodel->polygon=(POLYGON *)malloc(sizeof(POLYGON)*lmodel->numpolygon);
								memset(lmodel->polygon,0,sizeof(POLYGON)*lmodel->numpolygon);
							}
							//add info fprintf
						break;
						case MODFILE_NUMTEXTURES:
							//next arg is Num of textures
							param=strtok(NULL,"\n");
							lmodel->numtexture=atoi(param);
							if (lmodel->numpolygon>0) {
								//allocate space for each texture
								lmodel->texture=(TEXTUREMAP *)(sizeof(TEXTUREMAP)*lmodel->numtexture);
							}
							//add info fprintf
						break;
						case MODFILE_AXES:
							//read in axes
							for(j=0;j<lmodel->numaxis;j++) {								
								F3DGetLineFromText(tline,buf+CurChar);
								CurChar+=strlen(tline);
								CurLine++; //advance to next line
								param = strtok(tline, " "); //skip AXIS:
								param=strtok(NULL," "); //skip #:
								for(k=0;k<3;k++) {
									param=strtok(NULL,","); //x
									lmodel->axis[j].m.x[k]=atof(param);
								}
								for(k=0;k<3;k++) {
									param=strtok(NULL,","); //y
									lmodel->axis[j].m.y[k]=atof(param);
								}
								for(k=0;k<3;k++) {
									param=strtok(NULL,",\n"); //z
									lmodel->axis[j].m.z[k]=atof(param);
								}	
								F3DGetLineFromText(tline,buf+CurChar);
								CurChar+=strlen(tline);
								CurLine++; //advance to next line
								//and axis location
								param=strtok(tline," "); //skip AXIS_LOCATION:
								param=strtok(NULL,","); 
								lmodel->axis[j].loc.x=atof(param);
								param=strtok(NULL,","); 
								lmodel->axis[j].loc.y=atof(param);
								param=strtok(NULL,"\n"); 
								lmodel->axis[j].loc.z=atof(param);
								F3DGetLineFromText(tline,buf+CurChar);
								CurChar+=strlen(tline);
								CurLine++; //advance to next line
								//and NUMBER_OF_SUPERAXES: 
								param = strtok(tline, " "); //skip NUMBER_OF_SUPERAXES:
								OldFormat = 0;
								if (!strcmp(param, "NUMBER_OF_SUPERAXES:")) {
									param = strtok(NULL, ",\n");
									lmodel->axis[j].numsuperaxis = atoi(param);
								}
								else {
									if (!strcmp(param, "SUPERAXES:") || !strcmp(param, "SUBAXES:")) {
										//is an earlier format that only lists 20 SUPERAXES
										lmodel->axis[j].numsuperaxis = MAXSUPERAXES;
										OldFormat = 1;
									}
									else {
										sprintf(tstr, "Unknown AXIS section %s", param);
										FTMessageBox(tstr, FTMB_OK, "Free3D - Error", 0);
									}
								}
								//and SUPERAXES
								if (lmodel->axis[j].numsuperaxis > 0) {
									//allocate space for superaxes
									lmodel->axis[j].superaxis = (i32 *)malloc(sizeof(i32)*lmodel->axis[j].numsuperaxis);
									if (!OldFormat) { //was already skipped if OldFormat
										F3DGetLineFromText(tline, buf + CurChar);
										CurChar += strlen(tline);
										CurLine++; //advance to next line
										param = strtok(tline, " "); //skip SUPERAXES:
									}
									for (k = 0; k < lmodel->axis[j].numsuperaxis; k++) {
										param = strtok(NULL, ",\n");
										lmodel->axis[j].superaxis[k] = atoi(param);
									}
									if (OldFormat) {
										//recalculate the number of superaxes
										k = 0;
										while (k < lmodel->axis[j].numsuperaxis) {
											if (lmodel->axis[j].superaxis[k] < 0) {
												lmodel->axis[j].numsuperaxis = k;
												k = lmodel->axis[j].numsuperaxis; //exit while
												//no need to reallocate memory because it will just be free'd with free()
												//unless there are 0, then just free now
												if (k == 0) {
													free(lmodel->axis[j].superaxis);
												}
											}
										} //while
									} //if (OldFormat) {
								} //if (lmodel->axis[j].numsuperaxis > 0) {
								F3DGetLineFromText(tline,buf+CurChar);
								CurChar+=strlen(tline);
								CurLine++; //advance to next line

								//and NUMBER_OF_JOINTPOINTS
								param = strtok(tline, " "); //skip NUMBER_OF_JOINTPOINTS:
								if (!strcmp(param, "NUMBER_OF_JOINTPOINTS:")) {
									param = strtok(NULL, ",\n");
									lmodel->axis[j].numjointpoint = atoi(param);
								}
								else {
									sprintf(tstr, "Unknown AXIS section %s", param);
									FTMessageBox(tstr, FTMB_OK, "Free3D - Error", 0);
								}

								//and read in JOINT POINTS
//NUMBER_OF_JOINTPOINTS: 2
//0: -190,838,0
//1: -190,588,0
								if (lmodel->axis[j].numjointpoint > 0) {
									//allocate space for joint points
									lmodel->axis[j].jp = (POINT3D *)malloc(sizeof(POINT3D)*lmodel->axis[j].numjointpoint);
									lmodel->axis[j].ajp = (POINT3D *)malloc(sizeof(POINT3D)*lmodel->axis[j].numjointpoint);
									for (k = 0; k < lmodel->axis[j].numjointpoint; k++) {
										F3DGetLineFromText(tline,buf+CurChar);
										CurChar+=strlen(tline);
										CurLine++; //advance to next line

										param=strtok(tline," "); //skip #: 	
										param=strtok(NULL,","); 
										lmodel->axis[j].jp[k].x=atof(param);
										param=strtok(NULL,","); 
										lmodel->axis[j].jp[k].y=atof(param);
										param=strtok(NULL,"\n"); 
										lmodel->axis[j].jp[k].z=atof(param);

									} //for k
								} //if (lmodel->axis[j].numjoint > 0) {
							} //for j
							//add info fprintf							
						break;  //MODFILE_AXES
						case MODFILE_JOINTS:
							//read in joint pairs
//JOINTPAIRS:
//0: 1,2,0,0  
							for(j=0;j<lmodel->numjoint;j++) {								
								F3DGetLineFromText(tline,buf+CurChar);
								CurChar+=strlen(tline);
								CurLine++; //advance to next line
								param=strtok(tline," "); //skip #:
								param=strtok(NULL,","); //axis 0
								lmodel->joint[j].a[0]=atoi(param);
								param=strtok(NULL,","); //axis 1
								lmodel->joint[j].a[1]=atoi(param);
								param=strtok(NULL,","); //joint point 0
								lmodel->joint[j].p[0]=atoi(param);
								param=strtok(NULL,","); //joint point 1
								lmodel->joint[j].p[1]=atoi(param);
							} //for j
							//add info fprintf							
						break; //MODFILE_JOINTS
						case MODFILE_POINTS:
							//read in points
							for(j=0;j<lmodel->numpoint;j++) {
								F3DGetLineFromText(tline,buf+CurChar);
								CurChar+=strlen(tline);
								CurLine++; //advance to next line
								param=strtok(tline," "); //skip #:
								param=strtok(NULL,","); //x
								lmodel->point[j].x=atof(param);
								param=strtok(NULL,","); //y
								lmodel->point[j].y=atof(param);
								param=strtok(NULL,",\n"); //z
								lmodel->point[j].z=atof(param);
							} //for j
							//add info fprintf
						break;
						case MODFILE_NORMALS:
							//read in point normals
							//without knowing the shape the normals for a point on a model cannot be calculated
							for(j=0;j<lmodel->numnormal;j++) {
								F3DGetLineFromText(tline,buf+CurChar);
								CurChar+=strlen(tline);
								CurLine++; //advance to next line
								param=strtok(tline," "); //skip #:
								param=strtok(NULL,","); //x
								lmodel->normal[j].x=atof(param);
								param=strtok(NULL,","); //y
								lmodel->normal[j].y=atof(param);
								param=strtok(NULL,",\n"); //z
								lmodel->normal[j].z=atof(param);
							} //for j
							//add info fprintf
						break;
						case MODFILE_LINES:
							//read in lines
							for(j=0;j<lmodel->numline;j++) {
								F3DGetLineFromText(tline,buf+CurChar);
								CurChar+=strlen(tline);
								CurLine++; //advance to next line
								param=strtok(tline,"\n"); //skip LINE #:
								F3DGetLineFromText(tline,buf+CurChar);
								CurChar+=strlen(tline);
								CurLine++; //advance to next line
								param=strtok(tline,"("); //skip LINE_POINTS: (
								param=strtok(NULL,")");//number of points in polygon
								//lmodel->line[j].numpoint=atoi(param);
								for(k=0;k<2;k++) {  //read in point #s
									param=strtok(NULL,",\n"); 									
									lmodel->line[j].p[k]=atoi(param);
								}
								F3DGetLineFromText(tline,buf+CurChar);
								CurChar+=strlen(tline);
								CurLine++; //advance to next line
								param=strtok(tline," "); //skip LINE_COLOR:
								for(k=0;k<2;k++) {  //read in color of each point in polygon (in hex)
									param=strtok(NULL,",\n"); 
									lmodel->line[j].color[k]=strtol(param,0,16);
								}
							} //for j
							//add info fprintf
						break;
						case MODFILE_POLYGONS:
							//read in polygons until "TEXTURES:" is found
							//skip POLYGONS: (numpolygons)
							j=-1; //current polygon
							DoneReadingPolygons=0;
							while (!DoneReadingPolygons) {
							//for(j=0;j<lmodel->numpolygon;j++) {								
							//get a line
								F3DGetLineFromText(tline,buf+CurChar);
								tlinelen=strlen(tline); //preserve for moving back a line when finding end of polygon section ("TEXTURES:")
								CurChar+=tlinelen;
								CurLine++; //advance to next line
								param=strtok(tline," "); //get first parameter from line								
								//read in each polygon section until finding "POLYGON"

								polymatch=0;
								CurrentPolygonSection=0;
								while(polymatch==0 && CurrentPolygonSection<NUMPOLYGONSECTIONS) {
									if (!_stricmp(param,PolygonSection[CurrentPolygonSection])) {  
										//found a match
										polymatch=1;
										switch(CurrentPolygonSection) {
											case MODFILE_POLYGON_POLYGON: //POLYGON #:- must preceed any polygon info
												j++; //CurrentPolygon++
												if (lmodel->numnormal==0) {	
													//no point normals, tell the renderer to use the polygon normal for lighting polygon vertices
													lmodel->polygon[j].flags|=F3D_POLYGON_USE_POLYGON_NORMAL;
												}
											break;
											case MODFILE_POLYGON_FLAGS: //FLAGS:
												param=strtok(NULL,"|\n"); //get flag
												while(param!=0) {
													parammatch=0;
													//param[strlen(param)+1]=0; //need termination for strcmp
													//for some reason needs strncmp- above line doesn't work
													if (!strncmp(param,"F3D_POLYGON_USE_POLYGON_NORMAL",30)) {
														lmodel->polygon[j].flags|=F3D_POLYGON_USE_POLYGON_NORMAL;
														parammatch=1;
													}
													if (!parammatch) {
														sprintf(tstr,"Unknown polygon flag %s",param);
														FTMessageBox(tstr,FTMB_OK,"FREE3D - Error",0);
													}
													param=strtok(NULL,"|"); //get next param
												} //while(param!=0)												
											break;
											case MODFILE_POLYGON_AXIS: //AXIS: 
												param=strtok(NULL,"\n"); //get #
												lmodel->polygon[j].axis=atoi(param);
											break;
											case MODFILE_POLYGON_POINTS: //POINTS: (4) 0,1,2,3
												param=strtok(NULL,")");//number of points in polygon
												for(k=0;k<strlen(param);k++) { //no better way to do this I know of
													param[k]=param[k+1];
												}
												lmodel->polygon[j].numpoint=atoi(param);
												po=&lmodel->polygon[j];
												for(k=0;k<po->numpoint;k++) {  //read in point #s
													param=strtok(NULL,",\n"); 
													lmodel->polygon[j].p[k]=atoi(param);
												}
												//for each of the points set the axis the point belongs to in the pointaxis array
												for(k=0;k<po->numpoint;k++) {  
														lmodel->pointaxis[po->p[k]]=po->axis;
													} //for(i=0;i<lmodel-numpoint;i++) {
											break;
											case MODFILE_POLYGON_U: //U: 0.000,1.000,1.000,0.000
												for(k=0;k<po->numpoint;k++) {  //read in u's
													param=strtok(NULL,",\n"); 
													lmodel->polygon[j].u[k]=atof(param);
												}
											break;
											case MODFILE_POLYGON_V: //V: 0.000,0.000,1.000,1.000
												for(k=0;k<po->numpoint;k++) {  //read in v's
													param=strtok(NULL,",\n"); 
													lmodel->polygon[j].v[k]=atof(param);
												}
											break;
											case MODFILE_POLYGON_COLOR: //COLOR: 0xffffff,0xffffff,...
												for(k=0;k<po->numpoint;k++) {  //read in color of each point in polygon (in hex)
													param=strtok(NULL,",\n"); 
													lmodel->polygon[j].color[k]=strtol(param,0,16);
												}
											break;
											case MODFILE_POLYGON_NORMAL: //NORMAL:
												//NORMAL: x,y,z
												param=strtok(NULL,","); 
												lmodel->polygon[j].n.x=atof(param);
												param=strtok(NULL,","); 
												lmodel->polygon[j].n.y=atof(param);
												param=strtok(NULL,"\n"); 
												lmodel->polygon[j].n.z=atof(param);
											break;
											case MODFILE_POLYGON_TEXTURE: //TEXTURE:
												param=strtok(NULL,"\n"); //texture #
												lmodel->polygon[j].ti=atoi(param);
											break;
											case MODFILE_POLYGON_TRANSPARENCY: //TRANSPARENCY:
												param=strtok(NULL,"\n"); //transparency value
												lmodel->polygon[j].transparency=atof(param);
											break;
											case MODFILE_POLYGON_END: //TEXTURES:
												DoneReadingPolygons=1; //found "TEXTURES:" section
												//move back a line in the buffer to get it again												
												CurChar-=tlinelen;
												CurLine--;
											break;
											default: //unknown POLYGON SECTION
												sprintf(tstr,"Unknown POLYGON section number %d",CurrentPolygonSection);
												FTMessageBox(tstr,FTMB_OK,"FREE3D - Error",0);
											break;
										} //switch(CurrentPolygonSection)									
									} else {
										CurrentPolygonSection++;
									} //if (!_stricmp(param,PolygonSection[k])) {  
								} //while(polymatch==0 && CurrentPolygonSection<NUMCONFIGSECTIONS) {
								if (!polymatch) {
									sprintf(tstr,"Unknown POLYGON section %s",param);
									FTMessageBox(tstr,FTMB_OK,"FREE3D - Error",0);
								} //if (!polymatch) {
//							} //for j = each polygon entry
							} //while (!DoneReadingPolygons) {
							//add info fprintf
						break;
						case MODFILE_TEXTURES:
							//read in texture bitmap file names and load image data
							lmodel->texture=(TEXTUREMAP *)malloc(sizeof(TEXTUREMAP)*lmodel->numtexture);
							memset(lmodel->texture,0,sizeof(TEXTUREMAP)*lmodel->numtexture);
							for(j=0;j<lmodel->numtexture;j++) {
								F3DGetLineFromText(tline,buf+CurChar);
								CurChar+=strlen(tline);
								CurLine++; //advance to next line
								param=strtok(tline," "); //skip #:
								param=strtok(NULL,"\n"); //texture name
								strcpy(lmodel->texture[j].name,param);  
								//try to load texture file - name can include path, but usually is a relative path
								LoadTexture(lmodel,j,lmodel->texture[j].name);
							} //for j
							//add info fprintf
						break;
						default:
							sprintf(tstr,"Unknown section '%s' on line %d in model file %s",param,CurLine,filename);
							FTMessageBox(tstr,FTMB_OK,"Error",0);
							//i=NUMMODELFILESECTIONS;  //exit while loop
							//CurLine=NumLines; //exit outer each-line loop
						break;
					} //switch(i)
				} //if (!stricmp
				i++;  //advance to next section
			} //while(match==0
			if (!match) {
				sprintf(tstr,"Unknown section '%s' on line %d in model file %s",param,CurLine,filename);
				FTMessageBox(tstr,FTMB_OK,"Info",0);
				//i=NUMSECTIONS;  //exit while loop
				//CurLine=NumLines; //exit outer each-line loop
			}
		} //while(CurLine<NumLInes) 

	} else { //fptr!=0
		sprintf(tstr,"Could not open file '%s'\n",filename);
		FTMessageBox(tstr,FTMB_OK,"Error",0);
		fprintf(stderr,"Could not open file '%s'\n",filename);
		return(0);
	} //fptr!=0


	if (lmodel!=0) {




		//add a list (of indices) of all points that are on an axis to each axis (makes rendering faster- because axis does not need to be retrieved for each point)
		for(i=0;i<lmodel->numaxis;i++) {
			//for all model points- first count how many points are on each axis to know how much memory to allocate
			pa=lmodel->pointaxis;			
			count=0;
			for(j=0;j<lmodel->numpoint;j++) { 
				if (pa[j]==i) {
					count++;
				} //if (pa[j]==i) {
			} //for(j=0;j<lmodel->numpoint;j++) {
			axis=&lmodel->axis[i]; //get axis
			axis->numpoint=count;
			if (count>0) {
				axis->p=(int *)malloc(sizeof(int)*count); //allocate memory
				//now fill point array with point indices
				count=0;
				for(j=0;j<lmodel->numpoint;j++) { 
					if (pa[j]==i) {
						axis->p[count]=j; //add index to point to axis point index array
						count++;
					} //if (pa[j]==i) {
				} //for(j=0;j<lmodel->numpoint;j++) {
			} //if (count>0) {
		} //for(i=0;i<lmodel->numaxis;i++) {


		//may want to check to see if a model with this same name is already there, and rename if so
		//add this model to the global model list
		AddModelToModelList(lmodel);
	}

	return(lmodel);
} //MODEL *LoadModel(char *filename) {

int F3DGetLineFromText(char *tline,char *buf)
{
	int i,len;

	len=strlen(buf);
	i=0;
	while(i<len) {
		if (buf[i]==10) {
			tline[i]=10; //include chr(1)?
			tline[i+1]=0; //terminate string  
			return(i);
		} else {
			tline[i]=buf[i];
		}
		i++;
	} //while

	return(len);  //no chr(10) in string
} //int F3DGetLineFromText(char *tline,char *buf)


int UnloadModel(MODEL *lmodel) 
{
	if (lmodel!=0) {
		RemoveModelFromScene(lmodel);
		RemoveModelFromModelList(lmodel);
		FreeModel(lmodel);
		return(1);
	} //if (lmodel!=0) {
	return(0);
} //int UnloadModel(MODEL *lmodel) 


void LoadTexture(MODEL *lmodel,int ti,char *name)
{
unsigned char *tbmap;
//char TPath[256];

//Perhaps I should just load the bitmap and use the bmpihead values

//todo: add option where texturemap is a path to the bitmap

/*
//strcpy(TPath,Path);
strcpy(TPath,F3DStatus.Path);
#if Linux
strcat(TPath,"/texturemaps/");
#endif
#if WIN32
strcat(TPath,"\\texturemaps\\");
#endif
*/

//strcpy(path,TPath);
 //strcat(path,ltexture[h].name);  //add path
//strcat(TPath,name);  //add name to path

//if we are swapping a texture map that already exists
//delete old one and load new one as usual
if (lmodel->texture[ti].bmp!=0) {
	//interesting that free only needs 
	//first address of malloc'd memory to free memory
	//tptr=(unsigned char *)lmodel->texture[ti];
	//free(tptr);   //free bitmap pointer
	free(lmodel->texture[ti].bmp);
	lmodel->texture[ti].bmp=0;
}


//texture can be:
//1) bmp file
//2) avi file
//3) camera
//check to see if texture is a camera or video - if yes, it is a texture that is updated with each frame
//for an avi video texture map create the buffer and load the first bitmap image
//loading of rest of the bitmap images will happen in real-time
	if (_stricmp(((char *)name+strlen(name)-3),"avi")==0) {
		//avi
		//strcat(TPath,name);  //add path
		//tbmap=LoadVideoBitmap(lmodel,ti,TPath);  //allocates bitmap space at address

		//lmodel->texture[ti].bmp=(unsigned int *)tbmap;

	} else {
			if (name[0]=='@' || name[0]=='!') {  //camera texture
				//texture is camera
/*				
				strcpy(tstr,&name[1]);
				lcamera=GetCamera(tstr);
				//possibly attach bitmap to camera so other models can use it as a texture map
				tbmap=(unsigned char *)malloc(sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+WIDTH*HEIGHT*4); //freed in RenderSceneFromCamera?
				bmf=(BITMAPFILEHEADER *)tbmap;
				bmi=(BITMAPINFOHEADER *)((u8 *)tbmap+sizeof(BITMAPFILEHEADER));
				bmi->biWidth=WIDTH; 
				bmi->biHeight=HEIGHT;
#if BIT32
				bmi->biBitCount=32;
#else
				bmi->biBitCount=24;
#endif
				lmodel->texture[ti]=(unsigned int *)tbmap;
				//tph: below line needs to be fixed
				//strcpy(lmodel->texturename+TEXTURENAMESIZE*ti,name);
				//bitmap returned can be either upside-down or right-side-up
				//we need to have it upside down because it is a texture and bitmaps are upside down
				RenderSceneFromCamera((u8 *)tbmap,WIDTH,HEIGHT,lcamera,1); //render scene to screen bitmap from the current camera's perspective
				if (name[0]=='!') {  //make transparent Ellipse over camera texture
					//DrawTransparentEllipse((u8 *)tbmap);
				} //name[0]=='!'
				//indicate that we have already rendered this texturemap for this frame so we don't need to render again during the same frame is another model uses this texture
				//MODEL_CAMERA_ALREADY_RENDERED;
*/
			} else { 
				if (name[0]=='$') {  //make transparent Ellipse over non-camera texture
					/*
					strcat(TPath,&name[1]);  //add path					
					tbmap=F3DLoadBitmap(TPath);  //allocates bitmap space at address
					lmodel->texture[ti].bmp=(unsigned char *)tbmap;
					bmf=(BITMAPFILEHEADER *)tbmap;
					bmi=(BITMAPINFOHEADER *)((u8 *)tbmap+sizeof(BITMAPFILEHEADER));
					

	//				DrawTransparentEllipse((u8 *)tbmap);
					//texture is camera

					*/
				} else {//name[0]=='!'
					//strcat(TPath,name);  //add path
					//tbmap=F3DLoadBitmap(TPath);	//allocates bitmap memory
					tbmap=F3DLoadBitmap(name);	//allocates bitmap memory
					if (tbmap!=0) {
						lmodel->texture[ti].bmp=(BITMAPFILEHEADER *)tbmap;	//allocates bitmap memory
						lmodel->texture[ti].bmpinfo=(BITMAPINFOHEADER *)((unsigned char *)tbmap+sizeof(BITMAPFILEHEADER));
						//lmodel->texture[ti].bmpdata=((unsigned char *)tbmap+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER));
						//use biSize, because sometime there is colorspace info so sizeof(BITMAPINFOHEADER) is not enough to get to the pixel data
						lmodel->texture[ti].bmpdata=((unsigned char *)tbmap+sizeof(BITMAPFILEHEADER)+lmodel->texture[ti].bmpinfo->biSize); //sizeof(BITMAPINFOHEADER));
					} //if (tbmap!=0) {
				} ////name[0]=='!'
			} //if (tevent->name[0]=='@') {
	}

}  //void LoadTexture(MODEL *lmodel,int ti,char *name)

unsigned char * F3DLoadBitmap(char *filename)
{
FILE *fptr;
unsigned char * bmp;
char tstr[255];
int fSize,result;


fptr=fopen(filename,"rb");
if (fptr==0 ) {
	sprintf(tstr,"File Open %s Failed.\n",filename);
	FTMessageBox(tstr,FTMB_OK,"FREE3D - Error",0);
	return 0;
} else {
	// determine file size:
#if Linux
		fseeko(fptr,0,SEEK_END);
		fSize=ftello(fptr);
#endif
#if WIN32
		_fseeki64(fptr,0,SEEK_END);
		fSize=_ftelli64(fptr);
#endif	
		rewind(fptr);

	// allocate memory to contain the whole file:
	bmp=(unsigned char *)malloc(fSize);
	if (bmp == NULL) {
		sprintf(tstr,"Allocating memory for texturemap file '%s' failed.\n",filename);
		FTMessageBox(tstr,FTMB_OK,"Error",0);
		return(0);
	}
	//read in the entire file
	//copy the file into the buffer:
	result=fread(bmp,1,fSize,fptr);
	if (result != fSize) {
		sprintf(tstr,"Error reading model file %s\n",filename);
		FTMessageBox(tstr,FTMB_OK,"Error",0);
		return(0);
	}
	fclose(fptr);

return(bmp);
} //if fptr==0

} //F3DLoadBitmap

char *GetFileNameFromPath(char *filename) {
	int i;

	i=strlen(filename);
//	tstr[0]=0;
	while(i>0) {
		//exit once a / or \ separater has been found
#if Linux
		if (filename[i]==47) { // forward slash
#endif
#if WIN32
		if (filename[i]==92) {  //backward slash
#endif
			return(&filename[i+1]);
		} else {
			i--;
		}
	}
	//no folder separater found - return entire string
	return(filename);

} //char *GetFileNameFromPath(filename) {

int GetFileNameWithoutExtension(char *filename,char *dest) 
{
	int i;

	i=0;
	while(i<=strlen(filename)) {  //<= to get terminating 0 if no period in string
		//exit once a . has been found
		if (filename[i]==46) { // period
			dest[i]=0;
			return(1);
		} else {
			dest[i]=filename[i];
		}
		i++;
	}
	//no folder separater found - return entire string
	return(1);
} //char *GetFileNameWithoutExtension(filename) {

//fill a dropdown control with Model names
int UpdateModelList(FTControl *tcontrol)
{
	MODEL *lmodel;
	FTItem titem;

	if (tcontrol!=0) {
		//fill control with model names
		//free any existing itemlist
		DelFTItemList(tcontrol->ilist);
		lmodel=F3DStatus.imodel;
		while(lmodel!=0) {
			memset(&titem,0,sizeof(FTItem));
			strcpy(titem.name,lmodel->name);
			AddFTItem(tcontrol,&titem);
			lmodel=(MODEL *)lmodel->next;
		} //while
	} //if (tcontrol!=0)
return(1);
} //int UpdateModelList(FTControl *tcontrol)


int InitFree3D(void) 
{
#if Linux
	//create a mutex lock for RenderSceneFromCamerea() because otherwise when the model is moved in the middle of rendering there can be crashes.
   if (pthread_mutex_init(&F3DStatus.RenderLock, NULL) != 0)
    {
        printf("pthread_mutex_init of RenderLock failed\n");
        return 0;
    }
#endif //Linux

	return(1);
} //void InitFree3D(void)

int CloseFree3D(void) 
{
#if Linux
	//free the Render mutex lock
	pthread_mutex_destroy(&F3DStatus.RenderLock);
#endif

	return(1);
} //void CloseFree3D(void)
