#ifndef PROJECT_H
#define PROJECT_H

#include <QObject>
#include <TopoDS_Shape.hxx>
#include <TopTools_HSequenceOfShape.hxx>
#include <Bnd_Box.hxx>
class Component : public QObject
{
    Q_OBJECT
public:
    Component();
    Component(QString name,TopoDS_Shape shape);

    TopoDS_Shape getShape();
    Handle(TopTools_HSequenceOfShape) getSlice();
    QString getName();
    void setBox(Bnd_Box box);
    Bnd_Box getBox();
private:
    void setBox();
private:
    Handle(TopTools_HSequenceOfShape) mySlice;
    TopoDS_Shape myShape;
    QString name;
    Bnd_Box myBox;
};

#endif // PROJECT_H
