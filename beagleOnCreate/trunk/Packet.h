#ifndef PACKET_H
#define PACKET_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/*! \file Packet.h */

/*! The maximum packet size, used for buffer init. */
#define MAXPACKETSIZE 5000 
/*! The maximum number of ARtags that can be sent over at once. */
#define MAXARTAGSEEN 10
/*! The packet size when using PacketType SONAR. */
#define SONAR_PACKET_SIZE 24
/*! The packet size when using PacketType DATA. */
#define ARTAG_PACKET_SIZE MAXARTAGSEEN*20+12

/*!
 * 	\struct Packet Packet.h "Packet.h"
 *	\brief This struct defines the packet structure.
 */


/*! enum for the expected packet type */
enum PacketType
{
	/*! init packet should always be received first before any operation. */
	INIT = 1,
	/*! \deprecated end packet to end operation and allow other remote client to connect. */
	END,
	/*! ctrl packet to contrl any thing in this program. */
	CTRL,
	/*! data packet contains the artag id and pose info. */
	DATA,
	/*! image packet that has 160x120 image sent over. */
	IMAGE,
	/*! sonar packet contains the three sonar distance measurements. */
	SONAR,
	/*! error packet to report of any error (not implemented). */
	ERROR,
	/*! shutdown packet to quit this program entirely. */
	SHUTDOWN,
	/*! unknown?!. */
	UNKNOWN
};

struct Packet
{
	/*! The type of the packet \see PacketType */
	PacketType type;
	/*! The port number of remote client. */
	unsigned short port;
	/*! The address of the remote client. */
	struct in_addr addr;
	
	/*! This union contains different stuff depends on the PacketType. */
	union
	{
		struct
		{
			char data[256];
		}init;

		struct
		{
			char data[256];
		}end;

		struct
		{
			char data[256];
		}ctrl;

		struct
		{
			int tagId[MAXARTAGSEEN];
			float x[MAXARTAGSEEN];
			float y[MAXARTAGSEEN];
			float z[MAXARTAGSEEN];
			float yaw[MAXARTAGSEEN];
		}data;

		struct
		{
			int width;
			int height;
			char data[19200];
		}image;
		
		struct
		{
			float dist1;
			float dist2;
			float dist3;
		}sonar;

		struct
		{
			int errorCode;
			char data[256];
		}error;

		struct
		{
			char data[256];
		}shutdown;
		struct
		{
			char data[256];
		}unknown;
	}u;
};

#endif


