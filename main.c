// A box resting on a slowly turning platform slides outward when contact recycling is on.
//
// The platform is a kinematic box spinning about the vertical axis at a constant 0.12 rad/s.
// A 25 kg, 0.5 m box rests on it 1.75 m from the axis. With friction it should turn with the
// platform and stay put in the platform's frame. With contact recycling enabled it slowly
// slides outward; with recycling disabled it stays within a few millimetres.

#include "box3d/box3d.h"
#include "box3d/math_functions.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>

static b3Vec3 ToPlatformFrame( b3BodyId platform, b3BodyId box )
{
	b3Vec3 d = b3SubPos( b3Body_GetPosition( box ), b3Body_GetPosition( platform ) );
	return b3InvRotateVector( b3Body_GetRotation( platform ), d );
}

static float Run( const char* label, bool recycling, float spinRate, const char* recordingPath )
{
	b3WorldDef worldDef = b3DefaultWorldDef();
	worldDef.gravity = (b3Vec3){ 0.0f, -10.0f, 0.0f };
	b3WorldId world = b3CreateWorld( &worldDef );

	b3Recording* recording = NULL;
	if ( recordingPath != NULL )
	{
		recording = b3CreateRecording( 0 );
		b3World_StartRecording( world, recording );
	}

	b3Vec3 omega = { 0.0f, spinRate, 0.0f };

	b3BodyDef platformDef = b3DefaultBodyDef();
	platformDef.type = b3_kinematicBody;
	platformDef.angularVelocity = omega;
	platformDef.enableContactRecycling = recycling;
	b3BodyId platform = b3CreateBody( world, &platformDef );
	b3BoxHull platformHull = b3MakeBoxHull( 4.0f, 0.25f, 1.5f );
	b3ShapeDef platformShape = b3DefaultShapeDef();
	b3CreateHullShape( platform, &platformShape, &platformHull.base );

	// Start the box moving with the platform's surface so there is no initial slip.
	b3Vec3 start = { -1.5f, 0.5f, 0.9f };
	b3BodyDef boxDef = b3DefaultBodyDef();
	boxDef.type = b3_dynamicBody;
	boxDef.position = (b3Pos){ start.x, start.y, start.z };
	boxDef.linearVelocity = b3Cross( omega, start );
	boxDef.angularVelocity = omega;
	boxDef.enableSleep = false;
	b3BodyId box = b3CreateBody( world, &boxDef );
	b3BoxHull boxHull = b3MakeBoxHull( 0.25f, 0.25f, 0.25f );
	b3ShapeDef boxShape = b3DefaultShapeDef();
	boxShape.density = 25.0f / ( 0.5f * 0.5f * 0.5f );
	b3CreateHullShape( box, &boxShape, &boxHull.base );

	const float dt = 1.0f / 60.0f;
	const int subSteps = 4;

	// Let the box settle for one second, then measure for one minute.
	for ( int i = 0; i < 60; ++i )
	{
		b3World_Step( world, dt, subSteps );
	}

	b3Vec3 p0 = ToPlatformFrame( platform, box );
	float r0 = sqrtf( p0.x * p0.x + p0.z * p0.z );
	printf( "%-30s t %4.0f s  radius %.4f m\n", label, 0.0f, r0 );

	float r = r0;
	for ( int second = 1; second <= 60; ++second )
	{
		for ( int i = 0; i < 60; ++i )
		{
			b3World_Step( world, dt, subSteps );
		}
		b3Vec3 p = ToPlatformFrame( platform, box );
		r = sqrtf( p.x * p.x + p.z * p.z );
		if ( second % 10 == 0 )
		{
			printf( "%-30s t %4d s  radius %.4f m\n", label, second, r );
		}
	}

	b3Vec3 p1 = ToPlatformFrame( platform, box );
	float moved = sqrtf( ( p1.x - p0.x ) * ( p1.x - p0.x ) + ( p1.z - p0.z ) * ( p1.z - p0.z ) );
	printf( "%-30s moved %.1f cm in the platform frame over 60 s\n\n", label, 100.0f * moved );

	if ( recording != NULL )
	{
		b3World_StopRecording( world );
		if ( b3SaveRecordingToFile( recording, recordingPath ) )
		{
			printf( "saved a recording of this run to %s\n\n", recordingPath );
		}
		b3DestroyRecording( recording );
	}

	b3DestroyWorld( world );
	return moved;
}

// Pass a file path to save a Box3D recording of the first run (recycling on, 0.12 rad/s).
int main( int argc, char** argv )
{
	const char* recordingPath = argc > 1 ? argv[1] : NULL;
	Run( "recycling on,  0.12 rad/s", true, 0.12f, recordingPath );
	Run( "recycling off, 0.12 rad/s", false, 0.12f, NULL );
	Run( "recycling on,  0.06 rad/s", true, 0.06f, NULL );
	Run( "recycling off, 0.06 rad/s", false, 0.06f, NULL );
	return 0;
}
