// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/StringUtil.h>
#include <IceE/TraceUtil.h>
#include <IceE/Instance.h>
#include <IceE/DispatchStatus.h>
#include <IceE/Proxy.h>
#include <IceE/TraceLevels.h>
#include <IceE/Logger.h>
#include <IceE/BasicStream.h>
#include <IceE/Protocol.h>
#include <IceE/IdentityUtil.h>
#include <IceE/SafeStdio.h>
#include <IceE/OperationMode.h>

using namespace std;
using namespace IceE;
using namespace IceEInternal;

static void
printIdentityFacetOperation(string& s, BasicStream& stream)
{
    Identity identity;
    identity.__read(&stream);
    s += "\nidentity = ";
    s += identityToString(identity);

    vector<string> facet;
    stream.read(facet);
    s += "\nfacet = ";
    if(!facet.empty())
    {
        s += IceE::escapeString(facet[0], "");
    }

    string operation;
    stream.read(operation);
    s += "\noperation = ";
    s += operation;
}

static void
printRequestHeader(string& s, BasicStream& stream)
{
    printIdentityFacetOperation(s, stream);

    Byte mode;
    stream.read(mode);
    s += IceE::printfToString("\nmode = %d ", static_cast<int>(mode));
    switch(mode)
    {
	case Normal:
	{
	    s += "(normal)";
	    break;
	}

	case Nonmutating:
	{
	    s += "(nonmutating)";
	    break;
	}

	case Idempotent:
	{
	    s += "(idempotent)";
	    break;
	}

	default:
	{
	    s += "(unknown)";
	    break;
	}
    }

    Int sz;
    stream.readSize(sz);
    s += "\ncontext = ";
    while(sz--)
    {
	pair<string, string> pair;
	stream.read(pair.first);
	stream.read(pair.second);
	s += pair.first;
	s += "/";
	s += pair.second;
	if(sz)
	{
	    s += ", ";
	}
    }
}

static void
printHeader(string& s, BasicStream& stream)
{
    Byte magicNumber;
    stream.read(magicNumber);	// Don't bother printing the magic number
    stream.read(magicNumber);
    stream.read(magicNumber);
    stream.read(magicNumber);

    Byte pMajor;
    Byte pMinor;
    stream.read(pMajor);
    stream.read(pMinor);
    //IceE::printfToString("\nprotocol version = %d.%d", static_cast<unsigned>(pMajor),
    //static_cast<unsigned>(pMinor);

    Byte eMajor;
    Byte eMinor;
    stream.read(eMajor);
    stream.read(eMinor);
    //IceE::printfToString("\nencoding version = %d.%d", static_cast<unsigned>(eMajor),
    //static_cast<unsigned>(eMinor);

    Byte type;
    stream.read(type);
    s += IceE::printfToString("\nmessage type = %d ", static_cast<int>(type));

    switch(type)
    {
	case requestMsg:
	{
	    s += "(request)";
	    break;
	}

	case requestBatchMsg:
	{
	    s += "(batch request)";
	    break;
	}

	case replyMsg:
	{
	    s += "(reply)";
	    break;
	}

	case closeConnectionMsg:
	{
	    s += "(close connection)";
	    break;
	}

	case validateConnectionMsg:
	{
	    s += "(validate connection)";
	    break;
	}

	default:
	{
	    s += "(unknown)";
	    break;
	}
    }

    Byte compress;
    stream.read(compress);
    s += IceE::printfToString("\ncompression status = %d ", static_cast<int>(compress));

    switch(compress)
    {
	case 0:
	{
	    s += "(not compressed; do not compress response, if any)";
	    break;
	}

	case 1:
	{
	    s += "(not compressed; compress response, if any)";
	    break;
	}

	case 2:
	{
	    s += "(compressed; compress response, if any)";
	    break;
	}

	default:
	{
	    s += "(unknown)";
	    break;
	}
    }

    Int size;
    stream.read(size);
    s += IceE::printfToString("\nmessage size = %d", size);
}

void
IceEInternal::traceHeader(const char* heading, const BasicStream& str, const LoggerPtr& logger,
			 const TraceLevelsPtr& tl)
{
    if(tl->protocol >= 1)
    {
	BasicStream& stream = const_cast<BasicStream&>(str);
	BasicStream::Container::iterator p = stream.i;
	stream.i = stream.b.begin();

	string s(heading);
	printHeader(s, stream);

	logger->trace(tl->protocolCat, s);
	stream.i = p;
    }
}

void
IceEInternal::traceRequest(const char* heading, const BasicStream& str, const LoggerPtr& logger,
			  const TraceLevelsPtr& tl)
{
    if(tl->protocol >= 1)
    {
	BasicStream& stream = const_cast<BasicStream&>(str);
	BasicStream::Container::iterator p = stream.i;
	stream.i = stream.b.begin();

	string s(heading);
	printHeader(s, stream);

	Int requestId;
	stream.read(requestId);
	s += IceE::printfToString("\nrequest id = %d", requestId);
	if(requestId == 0)
	{
	    s += " (oneway)";
	}

	printRequestHeader(s, stream);

	logger->trace(tl->protocolCat, s);
	stream.i = p;
    }
}

#ifndef ICEE_NO_BATCH
void
IceEInternal::traceBatchRequest(const char* heading, const BasicStream& str, const LoggerPtr& logger,
			       const TraceLevelsPtr& tl)
{
    if(tl->protocol >= 1)
    {
	BasicStream& stream = const_cast<BasicStream&>(str);
	BasicStream::Container::iterator p = stream.i;
	stream.i = stream.b.begin();

	string s(heading);
	printHeader(s, stream);

	int batchRequestNum;
	stream.read(batchRequestNum);
	s += IceE::printfToString("\nnumber of requests = %d", batchRequestNum);

	for(int i = 0; i < batchRequestNum; ++i)
	{
	    s += IceE::printfToString("\nrequest #%d:", i);
	    printRequestHeader(s, stream);
	    stream.skipEncaps();
	}

	logger->trace(tl->protocolCat, s);
	stream.i = p;
    }
}
#endif

void
IceEInternal::traceReply(const char* heading, const BasicStream& str, const LoggerPtr& logger,
			const TraceLevelsPtr& tl)
{
    if(tl->protocol >= 1)
    {
	BasicStream& stream = const_cast<BasicStream&>(str);
	BasicStream::Container::iterator p = stream.i;
	stream.i = stream.b.begin();

	string s(heading);
	printHeader(s, stream);

	Int requestId;
	stream.read(requestId);
	s += IceE::printfToString("\nrequest id = %d", requestId);

	Byte status;
	stream.read(status);
	s += IceE::printfToString("\nreply status = %d ", static_cast<int>(status));
	switch(static_cast<DispatchStatus>(status))
	{
	    case DispatchOK:
	    {
		s += "(ok)";
		break;
	    }

	    case DispatchUserException:
	    {
		s += "(user exception)";
		break;
	    }

	    case DispatchObjectNotExist:
	    case DispatchFacetNotExist:
	    case DispatchOperationNotExist:
	    {
		switch(static_cast<DispatchStatus>(status))
		{
		    case DispatchObjectNotExist:
		    {
			s += "(object not exist)";
			break;
		    }

		    case DispatchFacetNotExist:
		    {
			s += "(facet not exist)";
			break;
		    }

		    case DispatchOperationNotExist:
		    {
			s += "(operation not exist)";
			break;
		    }

		    default:
		    {
			assert(false);
			break;
		    }
		}

		printIdentityFacetOperation(s, stream);
		break;
	    }

	    case DispatchUnknownException:
	    case DispatchUnknownLocalException:
	    case DispatchUnknownUserException:
	    {
		switch(static_cast<DispatchStatus>(status))
		{
		    case DispatchUnknownException:
		    {
			s += "(unknown exception)";
			break;
		    }

		    case DispatchUnknownLocalException:
		    {
			s += "(unknown local exception)";
			break;
		    }

		    case DispatchUnknownUserException:
		    {
			s += "(unknown user exception)";
			break;
		    }

		    default:
		    {
			assert(false);
			break;
		    }
		}
		
		string unknown;
		stream.read(unknown);
		s += "\nunknown = ";
		s += unknown;
		break;
	    }

	    default:
	    {
		s += "(unknown)";
		break;
	    }
	}

	logger->trace(tl->protocolCat, s);
	stream.i = p;
    }
}
