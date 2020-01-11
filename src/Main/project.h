#ifndef PROJECT_H
#define PROJECT_H

#include <QObject>
#include <TopTools_HSequenceOfShape.hxx>

class Project : public QObject
{
    Q_OBJECT
public:
    Project();
    Handle(TopTools_HSequenceOfShape) aHSequenceOfShape;
};

#endif // PROJECT_H
