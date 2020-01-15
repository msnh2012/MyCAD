#include "component.h"
#include "BRepBndLib.hxx"
Component::Component():name(""),myShape()
{

}

Component::Component(QString _name,TopoDS_Shape _shape)
{
    name = _name;
    myShape = _shape;
    mySlice = new TopTools_HSequenceOfShape();

    setBox();
}

Handle(TopTools_HSequenceOfShape) Component::getSlice()
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
