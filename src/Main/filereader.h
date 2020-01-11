#ifndef FILEREADER_H
#define FILEREADER_H
#include <TopTools_HSequenceOfShape.hxx>
class FileReader
{
public:
    FileReader();
    Standard_Integer ReadIGES(const Standard_CString& aFileName,Handle(TopTools_HSequenceOfShape)& aHSequenceOfShape);
};

#endif // FILEREADER_H
