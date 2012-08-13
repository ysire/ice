// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Test;

class TestObjectReader : public Ice::ObjectReader
{
public:
    virtual void
    read(const Ice::InputStreamPtr& in)
    {
        in->startObject();
        in->startSlice();
        in->endSlice();
        in->endObject(false);
    }
};

class BObjectReader : public Ice::ObjectReader
{
public:
    virtual void
    read(const Ice::InputStreamPtr& in)
    {
        in->startObject();
        // ::Test::B
        in->startSlice();
        Ice::Int v;
        in->read(v);
        in->endSlice();
        // ::Test::A
        in->startSlice();
        in->read(v);
        in->endSlice();
        in->endObject(false);
    }
};

class CObjectReader : public Ice::ObjectReader
{
public:
    virtual void
    read(const Ice::InputStreamPtr& in)
    {
        in->startObject();
        // ::Test::C
        in->startSlice();
        in->skipSlice();
        // ::Test::B
        in->startSlice();
        Ice::Int v;
        in->read(v);
        in->endSlice();
        // ::Test::A
        in->startSlice();
        in->read(v);
        in->endSlice();
        in->endObject(false);
    }
};

class DObjectWriter : public Ice::ObjectWriter
{
public:

    virtual void
    write(const Ice::OutputStreamPtr& out) const
    {
        out->startObject(0);
        // ::Test::D
        out->startSlice("::Test::D", false);
        string s = "test";
        out->write(s);
        IceUtil::Optional<vector<string> > o;
        o = vector<string>();
        o->push_back("test1");
        o->push_back("test2");
        o->push_back("test3");
        o->push_back("test4");
        out->write(1, o);
        APtr a = new A();
        a->mc = 18;
        out->write(1000, IceUtil::Optional<APtr>(a));
        out->endSlice();
        // ::Test::B
        out->startSlice(B::ice_staticId(), false);
        Ice::Int v = 14;
        out->write(v);
        out->endSlice();
        // ::Test::A
        out->startSlice(A::ice_staticId(), true);
        out->write(v);
        out->endSlice();
        out->endObject();
    }
};

class DObjectReader : public Ice::ObjectReader
{
public:

    virtual void
    read(const Ice::InputStreamPtr& in)
    {
        in->startObject();
        // ::Test::D
        in->startSlice();
        string s;
        in->read(s);
        test(s == "test");
        IceUtil::Optional<vector<string> > o;
        in->read(1, o);
        test(o && o->size() == 4 && 
             (*o)[0] == "test1" && (*o)[1] == "test2" && (*o)[2] == "test3" && (*o)[3] == "test4");
        in->read(1000, a);
        in->endSlice();
        // ::Test::B
        in->startSlice();
        Ice::Int v;
        in->read(v);
        in->endSlice();
        // ::Test::A
        in->startSlice();
        in->read(v);
        in->endSlice();
        in->endObject(false);
    }

    void check()
    {
        test((*a)->mc == 18);
    }

private:

    IceUtil::Optional<APtr> a;

};

class FactoryI : public Ice::ObjectFactory
{
    bool _enabled;

public:

    Ice::ObjectPtr
    create(const string& typeId) 
    {
        if(!_enabled)
        {
            return 0;
        }

        if(typeId == "::Test::OneOptional")
        {
            return new TestObjectReader;
        }
        else if(typeId == "::Test::MultiOptional")
        {
            return new TestObjectReader;
        }
        else if(typeId == "::Test::B")
        {
            return new BObjectReader;
        }
        else if(typeId == "::Test::C")
        {
            return new CObjectReader;
        }
        else if(typeId == "::Test::D")
        {
            return new DObjectReader;
        }
        
        return 0;
    }

    void destroy()
    {
    }

    void
    setEnabled(bool enabled)
    {
        _enabled = enabled;
    }
};

InitialPrx
allTests(const Ice::CommunicatorPtr& communicator, bool collocated)
{
    FactoryI* factory = new FactoryI();
    Ice::ObjectFactoryPtr f(factory);
    communicator->addObjectFactory(factory, "");

    cout << "testing stringToProxy... " << flush;
    string ref = "initial:default -p 12010";
    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    test(base);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    InitialPrx initial = InitialPrx::checkedCast(base);
    test(initial);
    test(initial == base);
    cout << "ok" << endl;

    cout << "testing constructor, copy constructor, and assignment operator... " << flush;

    OneOptionalPtr oo1 = new OneOptional();
    test(!oo1->a);
    oo1->a = 15;
    test(oo1->a && *oo1->a == 15);

    OneOptionalPtr oo2 = new OneOptional(16);
    test(oo2->a && *oo2->a == 16);

    OneOptionalPtr oo3 = new OneOptional(*oo2);
    test(oo3->a && *oo3->a == 16);

    *oo3 = *oo1;
    test(oo3->a && *oo3->a == 15);

    OneOptionalPtr oon = new OneOptional(IceUtil::None);
    test(!oon->a);

    MultiOptionalPtr mo1 = new MultiOptional();
    mo1->a = 15;
    mo1->b = true;
    mo1->c = 19;
    mo1->d = 78;
    mo1->e = 99;
    mo1->f = 5.5f;
    mo1->g = 1.0;
    mo1->h = "test";
    mo1->i = Test::MyEnumMember;
    mo1->j = MultiOptionalPrx::uncheckedCast(communicator->stringToProxy("test"));
    mo1->k = mo1;
    mo1->bs = ByteSeq(); 
    (*mo1->bs).push_back(5);
    mo1->ss = StringSeq();
    mo1->ss->push_back("test");
    mo1->ss->push_back("test2");
    mo1->iid = IntIntDict();
    (*mo1->iid)[4] = 3;
    mo1->sid = StringIntDict();
    (*mo1->sid)["test"] = 10;
    FixedStruct fs;
    fs.m = 78;
    mo1->fs = fs;
    VarStruct vs;
    vs.m = "hello";
    mo1->vs = vs;

    mo1->shs = ShortSeq();
    mo1->shs->push_back(1);
    mo1->es = MyEnumSeq();
    mo1->es->push_back(MyEnumMember);
    mo1->es->push_back(MyEnumMember);
    mo1->fss = FixedStructSeq();
    mo1->fss->push_back(fs);
    mo1->vss = VarStructSeq();
    mo1->vss->push_back(vs);
    mo1->oos = OneOptionalSeq();
    mo1->oos->push_back(oo1);
    mo1->oops = OneOptionalPrxSeq();
    mo1->oops->push_back(OneOptionalPrx::uncheckedCast(communicator->stringToProxy("test")));

    mo1->ied = IntEnumDict();
    mo1->ied.get()[4] = MyEnumMember;
    mo1->ifsd = IntFixedStructDict();
    mo1->ifsd.get()[4] = fs;
    mo1->ivsd = IntVarStructDict();
    mo1->ivsd.get()[5] = vs;
    mo1->iood = IntOneOptionalDict();
    mo1->iood.get()[5] = new OneOptional();
    mo1->iood.get()[5]->a = 15;
    mo1->ioopd = IntOneOptionalPrxDict();
    mo1->ioopd.get()[5] = OneOptionalPrx::uncheckedCast(communicator->stringToProxy("test"));

    mo1->bos = BoolSeq();
    mo1->bos->push_back(false);
    mo1->bos->push_back(true);
    mo1->bos->push_back(false);

    MultiOptionalPtr mo2 = new MultiOptional(*mo1);

    MultiOptionalPtr mo3 = new MultiOptional();
    *mo3 = *mo2;

    test(mo3->a == 15);
    test(mo3->b == true);
    test(mo3->c == 19);
    test(mo3->d == 78);
    test(mo3->e == 99);
    test(mo3->f == 5.5f);
    test(mo3->g == 1.0);
    test(mo3->h == string("test"));
    test(mo3->i == Test::MyEnumMember);
    test(mo3->j == MultiOptionalPrx::uncheckedCast(communicator->stringToProxy("test")));
    test(mo3->k == mo1);
    test(mo3->bs == mo1->bs);
    test(mo3->ss == mo1->ss);
    test(mo3->iid == mo1->iid);
    test(mo3->sid == mo1->sid);
    test(mo3->fs == mo1->fs);
    test(mo3->vs == mo1->vs);

    test(mo3->shs == mo1->shs);
    test(mo3->es == mo1->es);
    test(mo3->fss == mo1->fss);
    test(mo3->vss == mo1->vss);
    test(mo3->oos == mo1->oos);
    test(mo3->oops == mo1->oops);

    test(mo3->ied == mo1->ied);
    test(mo3->ifsd == mo1->ifsd);
    test(mo3->ivsd == mo1->ivsd);
    test(mo3->iood == mo1->iood);
    test(mo3->ioopd == mo1->ioopd);

    test(mo3->bos == mo1->bos);

    cout << "ok" << endl;

    cout << "testing marshalling... " << flush;
    OneOptionalPtr oo4 = OneOptionalPtr::dynamicCast(initial->pingPong(new OneOptional()));
    test(!oo4->a);

    OneOptionalPtr oo5 = OneOptionalPtr::dynamicCast(initial->pingPong(oo1));
    test(oo1->a == oo5->a);

    MultiOptionalPtr mo4 = MultiOptionalPtr::dynamicCast(initial->pingPong(new MultiOptional()));
    test(!mo4->a);
    test(!mo4->b);
    test(!mo4->c);
    test(!mo4->d);
    test(!mo4->e);
    test(!mo4->f);
    test(!mo4->g);
    test(!mo4->h);
    test(!mo4->i);
    test(!mo4->j);
    test(!mo4->k);
    test(!mo4->bs);
    test(!mo4->ss);
    test(!mo4->iid);
    test(!mo4->sid);
    test(!mo4->fs);
    test(!mo4->vs);

    test(!mo4->shs);
    test(!mo4->es);
    test(!mo4->fss);
    test(!mo4->vss);
    test(!mo4->oos);
    test(!mo4->oops);

    test(!mo4->ied);
    test(!mo4->ifsd);
    test(!mo4->ivsd);
    test(!mo4->iood);
    test(!mo4->ioopd);

    test(!mo4->bos);
    
    mo1->k = mo1;
    MultiOptionalPtr mo5 = MultiOptionalPtr::dynamicCast(initial->pingPong(mo1));
    test(mo5->a == mo1->a);
    test(mo5->b == mo1->b);
    test(mo5->c == mo1->c);
    test(mo5->d == mo1->d);
    test(mo5->e == mo1->e);
    test(mo5->f == mo1->f);
    test(mo5->g == mo1->g);
    test(mo5->h == mo1->h);
    test(mo5->i == mo1->i);
    test(mo5->j == mo1->j);
    test(mo5->k == mo5->k);
    test(mo5->bs == mo1->bs);
    test(mo5->ss == mo1->ss);
    test(mo5->iid == mo1->iid);
    test(mo5->sid == mo1->sid);
    test(mo5->fs == mo1->fs);
    test(mo5->vs == mo1->vs);

    test(mo5->shs == mo1->shs);
    test(mo5->es == mo1->es);
    test(mo5->fss == mo1->fss);
    test(mo5->vss == mo1->vss);
    test(!mo5->oos->empty() && (*mo5->oos)[0]->a == oo1->a);
    test(mo5->oops == mo1->oops);

    test(mo5->ied == mo1->ied);
    test(mo5->ifsd == mo1->ifsd);
    test(mo5->ivsd == mo1->ivsd);
    test(!mo5->iood->empty() && (*mo5->iood)[5]->a == 15);
    test(mo5->ioopd == mo1->ioopd);

    test(mo5->bos == mo1->bos);

    // Clear the first half of the optional parameters
    MultiOptionalPtr mo6 = new MultiOptional(*mo5);
    mo6->a = IceUtil::None;
    mo6->c = IceUtil::None;
    mo6->e = IceUtil::None;
    mo6->g = IceUtil::None;
    mo6->i = IceUtil::None;
    mo6->k = IceUtil::None;
    mo6->ss = IceUtil::None;
    mo6->sid = IceUtil::None;
    mo6->vs = IceUtil::None;

    mo6->es = IceUtil::None;
    mo6->vss = IceUtil::None;
    mo6->oops = IceUtil::None;

    mo6->ied = IceUtil::None;
    mo6->ivsd = IceUtil::None;
    mo6->ioopd = IceUtil::None;

    MultiOptionalPtr mo7 = MultiOptionalPtr::dynamicCast(initial->pingPong(mo6));
    test(!mo7->a);
    test(mo7->b == mo1->b);
    test(!mo7->c);
    test(mo7->d == mo1->d);
    test(!mo7->e);
    test(mo7->f == mo1->f);
    test(!mo7->g);
    test(mo7->h == mo1->h);
    test(!mo7->i);
    test(mo7->j == mo1->j);
    test(!mo7->k);
    test(mo7->bs == mo1->bs);
    test(!mo7->ss);
    test(mo7->iid == mo1->iid);
    test(!mo7->sid);
    test(mo7->fs == mo1->fs);
    test(!mo7->vs);

    test(mo7->shs == mo1->shs);
    test(!mo7->es);
    test(mo7->fss == mo1->fss);
    test(!mo7->vss);
    test(!mo7->oos->empty() && (*mo7->oos)[0]->a == oo1->a);
    test(!mo7->oops);

    test(!mo7->ied);
    test(mo7->ifsd == mo1->ifsd);
    test(!mo7->ivsd);
    test(!mo7->iood->empty() && (*mo7->iood)[5]->a == 15);
    test(!mo7->ioopd);

    // Clear the second half of the optional parameters
    MultiOptionalPtr mo8 = new MultiOptional(*mo5);
    mo8->b = IceUtil::None;
    mo8->d = IceUtil::None;
    mo8->f = IceUtil::None;
    mo8->h = IceUtil::None;
    mo8->j = IceUtil::None;
    mo8->bs = IceUtil::None;
    mo8->iid = IceUtil::None;
    mo8->fs = IceUtil::None;

    mo8->shs = IceUtil::None;
    mo8->fss = IceUtil::None;
    mo8->oos = IceUtil::None;

    mo8->ifsd = IceUtil::None;
    mo8->iood = IceUtil::None;

    MultiOptionalPtr mo9 = MultiOptionalPtr::dynamicCast(initial->pingPong(mo8));
    test(mo9->a == mo1->a);
    test(!mo9->b);
    test(mo9->c == mo1->c);
    test(!mo9->d);
    test(mo9->e == mo1->e);
    test(!mo9->f);
    test(mo9->g == mo1->g);
    test(!mo9->h);
    test(mo9->i == mo1->i);
    test(!mo9->j);
    test(mo9->k == mo9->k);
    test(!mo9->bs);
    test(mo9->ss == mo1->ss);
    test(!mo9->iid);
    test(mo9->sid == mo1->sid);
    test(!mo9->fs);
    test(mo9->vs == mo1->vs);

    test(!mo8->shs);
    test(mo8->es == mo1->es);
    test(!mo8->fss);
    test(mo8->vss == mo1->vss);
    test(!mo8->oos);
    test(mo8->oops == mo1->oops);

    test(mo8->ied == mo1->ied);
    test(!mo8->ifsd);
    test(mo8->ivsd == mo1->ivsd);
    test(!mo8->iood);

    //
    // Send a request using blobjects. Upon receival, we don't read
    // any of the optional members. This ensures the optional members
    // are skipped even if the receiver knows nothing about them.
    //
    factory->setEnabled(true);
    Ice::OutputStreamPtr out = Ice::createOutputStream(communicator);
    out->startEncapsulation();
    out->write(oo1);
    out->endEncapsulation();
    Ice::ByteSeq inEncaps;
    out->finished(inEncaps);
    Ice::ByteSeq outEncaps;
    test(initial->ice_invoke("pingPong", Ice::Normal, inEncaps, outEncaps));
    Ice::InputStreamPtr in = Ice::createInputStream(communicator, outEncaps);
    in->startEncapsulation();
    Ice::ObjectPtr obj;
    in->read(obj);
    in->endEncapsulation();
    test(obj && dynamic_cast<TestObjectReader*>(obj.get()));

    out = Ice::createOutputStream(communicator);
    out->startEncapsulation();
    out->write(mo1);
    out->endEncapsulation();
    out->finished(inEncaps);
    test(initial->ice_invoke("pingPong", Ice::Normal, inEncaps, outEncaps));
    in = Ice::createInputStream(communicator, outEncaps);
    in->startEncapsulation();
    in->read(obj);
    in->endEncapsulation();
    test(obj && dynamic_cast<TestObjectReader*>(obj.get()));
    factory->setEnabled(false);
    cout << "ok" << endl;

    cout << "testing marshalling of large containers with fixed size elements..." << flush;
    MultiOptionalPtr mc = new MultiOptional();

    ByteSeq byteSeq;
    byteSeq.resize(1000);
    mc->bs = byteSeq;

    ShortSeq shortSeq;
    shortSeq.resize(300);
    mc->shs = shortSeq;

    FixedStructSeq fsSeq;
    fsSeq.resize(300);
    mc->fss = fsSeq;

    IntFixedStructDict ifsd;
    for(int i = 0; i < 300; ++i)
    {
        ifsd.insert(make_pair(i, FixedStruct()));
    }
    mc->ifsd = ifsd;

    mc = MultiOptionalPtr::dynamicCast(initial->pingPong(mc));
    test(mc->bs->size() == 1000);
    test(mc->shs->size() == 300);
    test(mc->fss->size() == 300);
    test(mc->ifsd->size() == 300);

    factory->setEnabled(true);
    out = Ice::createOutputStream(communicator);
    out->startEncapsulation();
    out->write(mc);
    out->endEncapsulation();
    out->finished(inEncaps);
    test(initial->ice_invoke("pingPong", Ice::Normal, inEncaps, outEncaps));
    in = Ice::createInputStream(communicator, outEncaps);
    in->startEncapsulation();
    in->read(obj);
    in->endEncapsulation();
    test(obj && dynamic_cast<TestObjectReader*>(obj.get()));
    factory->setEnabled(false);

    cout << "ok" << endl;

    cout << "testing tag marshalling... " << flush;
    BPtr b = new B();
    BPtr b2 = BPtr::dynamicCast(initial->pingPong(b));
    test(!b2->ma);
    test(!b2->mb);
    test(!b2->mc);
    
    b->ma = 10;
    b->mb = 11;
    b->mc = 12;
    b->md = 13;

    b2 = BPtr::dynamicCast(initial->pingPong(b));
    test(b2->ma == 10);
    test(b2->mb == 11);
    test(b2->mc == 12);
    test(b2->md == 13);

    factory->setEnabled(true);
    out = Ice::createOutputStream(communicator);
    out->startEncapsulation();
    out->write(b);
    out->endEncapsulation();
    out->finished(inEncaps);
    test(initial->ice_invoke("pingPong", Ice::Normal, inEncaps, outEncaps));
    in = Ice::createInputStream(communicator, outEncaps);
    in->startEncapsulation();
    in->read(obj);
    in->endEncapsulation();
    test(obj);
    factory->setEnabled(false);

    cout << "ok" << endl;

    cout << "testing optional with default values... " << flush;
    WDPtr wd = WDPtr::dynamicCast(initial->pingPong(new WD()));
    test(*wd->a == 5);
    test(*wd->s == "test");
    wd->a = IceUtil::None;
    wd->s = IceUtil::None;
    wd = WDPtr::dynamicCast(initial->pingPong(wd));
    test(!wd->a);
    test(!wd->s);
    cout << "ok" << endl;

    if(communicator->getProperties()->getPropertyAsInt("Ice.Default.SlicedFormat") > 0)
    {
        cout << "testing marshalling with unknown class slices... " << flush;
        {
            CPtr c = new C();
            c->ss = "test";
            c->ms = "testms";
            out = Ice::createOutputStream(communicator);
            out->startEncapsulation();
            out->write(c);
            out->endEncapsulation();
            out->finished(inEncaps);
            factory->setEnabled(true);
            test(initial->ice_invoke("pingPong", Ice::Normal, inEncaps, outEncaps));
            in = Ice::createInputStream(communicator, outEncaps);
            in->startEncapsulation();
            in->read(obj);
            in->endEncapsulation();
            test(dynamic_cast<CObjectReader*>(obj.get())); 
            factory->setEnabled(false);

            factory->setEnabled(true);
            out = Ice::createOutputStream(communicator);
            out->startEncapsulation();
            Ice::ObjectPtr d = new DObjectWriter();
            out->write(d);
            out->endEncapsulation();
            out->finished(inEncaps);
            test(initial->ice_invoke("pingPong", Ice::Normal, inEncaps, outEncaps));
            in = Ice::createInputStream(communicator, outEncaps);
            in->startEncapsulation();
            in->read(obj);
            in->endEncapsulation();
            test(obj && dynamic_cast<DObjectReader*>(obj.get()));
            dynamic_cast<DObjectReader*>(obj.get())->check();
            factory->setEnabled(false);
        }    
        cout << "ok" << endl;

        cout << "testing optionals with unknown classes..." << flush;
        {
            APtr a = new A();

            out = Ice::createOutputStream(communicator);
            out->startEncapsulation();
            out->write(a);
            out->write(1, IceUtil::makeOptional(Ice::ObjectPtr(new DObjectWriter)));
            out->endEncapsulation();
            out->finished(inEncaps);
            test(initial->ice_invoke("opClassAndUnknownOptional", Ice::Normal, inEncaps, outEncaps));

            in = Ice::createInputStream(communicator, outEncaps);
            in->startEncapsulation();
            in->endEncapsulation();
        }
        cout << "ok" << endl;
    }

    cout << "testing optional parameters... " << flush;
    {
        IceUtil::Optional<Ice::Byte> p1;
        IceUtil::Optional<Ice::Byte> p3;
        IceUtil::Optional<Ice::Byte> p2 = initial->opByte(p1, p3);
        test(!p2 && !p3);

        p1 = 56;
        p2 = initial->opByte(p1, p3);
        test(p2 == 56 && p3 == 56);

        out = Ice::createOutputStream(communicator);
        out->startEncapsulation();
        out->write(2, p1);
        out->endEncapsulation();
        out->finished(inEncaps);
        initial->ice_invoke("opByte", Ice::Normal, inEncaps, outEncaps);
        in = Ice::createInputStream(communicator, outEncaps);
        in->startEncapsulation();
        in->read(1, p2);
        in->read(3, p3);
        in->endEncapsulation();
        test(p2 == 56 && p3 == 56);

        in = Ice::createInputStream(communicator, outEncaps);
        in->startEncapsulation();
        in->endEncapsulation();
    }

    {
        IceUtil::Optional<Ice::Long> p1;
        IceUtil::Optional<Ice::Long> p3;
        IceUtil::Optional<Ice::Long> p2 = initial->opLong(p1, p3);
        test(!p2 && !p3);

        p1 = 56;
        p2 = initial->opLong(p1, p3);
        test(p2 == 56 && p3 == 56);

        out = Ice::createOutputStream(communicator);
        out->startEncapsulation();
        out->write(2, p1);
        out->endEncapsulation();
        out->finished(inEncaps);
        initial->ice_invoke("opLong", Ice::Normal, inEncaps, outEncaps);
        in = Ice::createInputStream(communicator, outEncaps);
        in->startEncapsulation();
        in->read(1, p2);
        in->read(3, p3);
        in->endEncapsulation();
        test(p2 == 56 && p3 == 56);

        in = Ice::createInputStream(communicator, outEncaps);
        in->startEncapsulation();
        in->endEncapsulation();
    }

    {
        IceUtil::Optional<string> p1;
        IceUtil::Optional<string> p3;
        IceUtil::Optional<string> p2 = initial->opString(p1, p3);
        test(!p2 && !p3);

        p1 = string("test");
        p2 = initial->opString(p1, p3);
        test(p2 == string("test") && p3 == string("test"));

        out = Ice::createOutputStream(communicator);
        out->startEncapsulation();
        out->write(2, p1);
        out->endEncapsulation();
        out->finished(inEncaps);
        initial->ice_invoke("opString", Ice::Normal, inEncaps, outEncaps);
        in = Ice::createInputStream(communicator, outEncaps);
        in->startEncapsulation();
        in->read(1, p2);
        in->read(3, p3);
        in->endEncapsulation();
        test(p2 == string("test") && p3 == string("test"));

        in = Ice::createInputStream(communicator, outEncaps);
        in->startEncapsulation();
        in->endEncapsulation();
    }

    {
        IceUtil::Optional<OneOptionalPtr> p1;
        IceUtil::Optional<OneOptionalPtr> p3;
        IceUtil::Optional<OneOptionalPtr> p2 = initial->opOneOptional(p1, p3);
        test(!p2 && !p3);

        p1 = new OneOptional(58);
        p2 = initial->opOneOptional(p1, p3);
        test((*p2)->a == 58 && (*p3)->a == 58);

        out = Ice::createOutputStream(communicator);
        out->startEncapsulation();
        out->write(2, p1);
        out->endEncapsulation();
        out->finished(inEncaps);
        initial->ice_invoke("opOneOptional", Ice::Normal, inEncaps, outEncaps);
        in = Ice::createInputStream(communicator, outEncaps);
        in->startEncapsulation();
        in->read(1, p2);
        in->read(3, p3);
        in->endEncapsulation();
        test((*p2)->a == 58 && (*p3)->a == 58);

        in = Ice::createInputStream(communicator, outEncaps);
        in->startEncapsulation();
        in->endEncapsulation();
    }

    {
        IceUtil::Optional<OneOptionalPrx> p1;
        IceUtil::Optional<OneOptionalPrx> p3;
        IceUtil::Optional<OneOptionalPrx> p2 = initial->opOneOptionalProxy(p1, p3);
        test(!p2 && !p3);

        p1 = OneOptionalPrx::uncheckedCast(communicator->stringToProxy("test"));
        p2 = initial->opOneOptionalProxy(p1, p3);
        test(p2 == p1 && p3 == p1);

        out = Ice::createOutputStream(communicator);
        out->startEncapsulation();
        out->write(2, p1);
        out->endEncapsulation();
        out->finished(inEncaps);
        initial->ice_invoke("opOneOptionalProxy", Ice::Normal, inEncaps, outEncaps);
        in = Ice::createInputStream(communicator, outEncaps);
        in->startEncapsulation();
        in->read(1, p2);
        in->read(3, p3);
        in->endEncapsulation();
        test(p2 == p1 && p3 == p1);

        in = Ice::createInputStream(communicator, outEncaps);
        in->startEncapsulation();
        in->endEncapsulation();
    }

    cout << "ok" << endl;

    cout << "testing optional parameters and custom sequences... " << flush;
    {
        IceUtil::Optional<std::pair<const Ice::Byte*, const Ice::Byte*> > p1;
        IceUtil::Optional<ByteSeq> p3;
        IceUtil::Optional<ByteSeq> p2 = initial->opByteSeq(p1, p3);
        test(!p2 && !p3);

        vector<Ice::Byte> bs(100);
        fill(bs.begin(), bs.end(), 56);
        p1 = make_pair(&bs[0], &bs[0] + bs.size());
        p2 = initial->opByteSeq(p1, p3);
        test(p2 && p3);
        test(p2 == bs && p3 == bs);

        out = Ice::createOutputStream(communicator);
        out->startEncapsulation();
        out->write(2, p1);
        out->endEncapsulation();
        out->finished(inEncaps);
        initial->ice_invoke("opByteSeq", Ice::Normal, inEncaps, outEncaps);
        in = Ice::createInputStream(communicator, outEncaps);
        in->startEncapsulation();
        in->read(1, p2);
        in->read(3, p3);
        in->endEncapsulation();
        test(p2 == bs && p3 == bs);

        in = Ice::createInputStream(communicator, outEncaps);
        in->startEncapsulation();
        in->endEncapsulation();
    }

    {
        IceUtil::Optional<std::pair<const Ice::Short*, const Ice::Short*> > p1;
        IceUtil::Optional<ShortSeq> p3;
        IceUtil::Optional<ShortSeq> p2 = initial->opShortSeq(p1, p3);
        test(!p2 && !p3);

        vector<Ice::Short> bs(100);
        fill(bs.begin(), bs.end(), 56);
        p1 = make_pair(&bs[0], &bs[0] + bs.size());
        p2 = initial->opShortSeq(p1, p3);
        test(p2 && p3);
        test(p2 == bs && p3 == bs);

        out = Ice::createOutputStream(communicator);
        out->startEncapsulation();
        out->write(2, p1);
        out->endEncapsulation();
        out->finished(inEncaps);
        initial->ice_invoke("opByteSeq", Ice::Normal, inEncaps, outEncaps);
        in = Ice::createInputStream(communicator, outEncaps);
        in->startEncapsulation();
        in->read(1, p2);
        in->read(3, p3);
        in->endEncapsulation();
        test(p2 == bs && p3 == bs);

        in = Ice::createInputStream(communicator, outEncaps);
        in->startEncapsulation();
        in->endEncapsulation();
    }

    {
        IceUtil::Optional<std::pair<const bool*, const bool*> > p1;
        IceUtil::Optional<BoolSeq> p3;
        IceUtil::Optional<BoolSeq> p2 = initial->opBoolSeq(p1, p3);
        test(!p2 && !p3);

        bool bs[100];
        vector<bool> bsv(&bs[0], &bs[0] + 100);
        p1 = make_pair(&bs[0], &bs[0] + 100);
        p2 = initial->opBoolSeq(p1, p3);
        test(p2 && p3);
        test(p2 == bsv && p3 == bsv);

        out = Ice::createOutputStream(communicator);
        out->startEncapsulation();
        out->write(2, p1);
        out->endEncapsulation();
        out->finished(inEncaps);
        initial->ice_invoke("opBoolSeq", Ice::Normal, inEncaps, outEncaps);
        in = Ice::createInputStream(communicator, outEncaps);
        in->startEncapsulation();
        in->read(1, p2);
        in->read(3, p3);
        in->endEncapsulation();
        test(p2 == bsv && p3 == bsv);

        in = Ice::createInputStream(communicator, outEncaps);
        in->startEncapsulation();
        in->endEncapsulation(); 
    }

    {
        IceUtil::Optional<std::pair<StringSeq::const_iterator, StringSeq::const_iterator> > p1;
        IceUtil::Optional<StringSeq> p3;
        IceUtil::Optional<StringSeq> p2 = initial->opStringSeq(p1, p3);
        test(!p2 && !p3);

        StringSeq ss(10);
        fill(ss.begin(), ss.end(), "test1");
        p1 = make_pair(&ss[0], &ss[0] + ss.size());
        p2 = initial->opStringSeq(p1, p3);
        test(p2 && p3);
        test(p2 == ss && p3 == ss);

        out = Ice::createOutputStream(communicator);
        out->startEncapsulation();
        out->write(2, p1);
        out->endEncapsulation();
        out->finished(inEncaps);
        initial->ice_invoke("opStringSeq", Ice::Normal, inEncaps, outEncaps);
        in = Ice::createInputStream(communicator, outEncaps);
        in->startEncapsulation();
        in->read(1, p2);
        in->read(3, p3);
        in->endEncapsulation();
        test(p2 == ss && p3 == ss);

        in = Ice::createInputStream(communicator, outEncaps);
        in->startEncapsulation();
        in->endEncapsulation();
    }

    {
        IceUtil::Optional<std::pair<const FixedStruct*, const FixedStruct*> > p1;
        IceUtil::Optional<FixedStructSeq> p3;
        IceUtil::Optional<FixedStructSeq> p2 = initial->opFixedStructSeq(p1, p3);
        test(!p2 && !p3);

        FixedStruct fss[10];
        vector<FixedStruct> fssv(&fss[0], &fss[0] + 10);
        p1 = make_pair(&fss[0], &fss[0] + 10);
        p2 = initial->opFixedStructSeq(p1, p3);
        test(p2 && p3);
        test(p2 == fssv && p3 == fssv);

        out = Ice::createOutputStream(communicator);
        out->startEncapsulation();
        out->write(2, p1);
        out->endEncapsulation();
        out->finished(inEncaps);
        initial->ice_invoke("opFixedStructSeq", Ice::Normal, inEncaps, outEncaps);
        in = Ice::createInputStream(communicator, outEncaps);
        in->startEncapsulation();
        in->read(1, p2);
        in->read(3, p3);
        in->endEncapsulation();
        test(p2 == fssv && p3 == fssv);

        in = Ice::createInputStream(communicator, outEncaps);
        in->startEncapsulation();
        in->endEncapsulation(); 
    }

    {
        IceUtil::Optional<std::pair<VarStructSeq::const_iterator, VarStructSeq::const_iterator> > p1;
        IceUtil::Optional<VarStructSeq> p3;
        IceUtil::Optional<VarStructSeq> p2 = initial->opVarStructSeq(p1, p3);
        test(!p2 && !p3);

        VarStructSeq ss(10);
        p1 = make_pair(&ss[0], &ss[0] + ss.size());
        p2 = initial->opVarStructSeq(p1, p3);
        test(p2 && p3);
        test(p2 == ss && p3 == ss);

        out = Ice::createOutputStream(communicator);
        out->startEncapsulation();
        out->write(2, p1);
        out->endEncapsulation();
        out->finished(inEncaps);
        initial->ice_invoke("opVarStructSeq", Ice::Normal, inEncaps, outEncaps);
        in = Ice::createInputStream(communicator, outEncaps);
        in->startEncapsulation();
        in->read(1, p2);
        in->read(3, p3);
        in->endEncapsulation();
        test(p2 == ss && p3 == ss);

        in = Ice::createInputStream(communicator, outEncaps);
        in->startEncapsulation();
        in->endEncapsulation();
    }

    cout << "ok" << endl;

    cout << "testing exception optionals... " << flush;
    {
        try
        {
            initial->opOptionalException(IceUtil::None, IceUtil::None, IceUtil::None);
        }
        catch(const OptionalException& ex)
        {
            test(!ex.a);
            test(!ex.b);
            test(!ex.o);
        }

        try
        {
            initial->opOptionalException(30, string("test"), OneOptionalPtr(new OneOptional(53)));
        }
        catch(const OptionalException& ex)
        {
            test(ex.a == 30);
            test(ex.b == string("test"));
            test((*ex.o)->a = 53);
        }
    }
    cout << "ok" << endl;

    return initial;
}