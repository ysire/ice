// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_DISPATCH_STATUS_H
#define ICEE_DISPATCH_STATUS_H

namespace IceEInternal
{

enum DispatchStatus
{
    DispatchOK,
    DispatchUserException,
    DispatchObjectNotExist,
    DispatchFacetNotExist,
    DispatchOperationNotExist,
    DispatchUnknownLocalException,
    DispatchUnknownUserException,
    DispatchUnknownException
};

}

#endif
