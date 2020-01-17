#include "component.h"
#include "BRepBndLib.hxx"
Component::Component():name(""),myShape()
{

}

Component::Component(QString _name,TopoDS_Shape _shape)
{
    name = _name;
    myShape = _shape;
    setBox();
}

void Component::setSlice(QVector<SliceData> slice)
{
    mySlice = slice;
}

QVector<SliceData> &Component::getSlice()
{
    return mySlice;
}

QString Component::getName()
{
    return name;
}

TopoDS_Shape Component::getShape()
{
    return myShape;
}

void Component::setBox(Bnd_Box box)
{
    myBox = box;
}

void Component::setBox()
{
    BRepBndLib::AddOptimal(myShape,myBox);
}
Bnd_Box Component::getBox()
{
    return myBox;
}
