#include "filereader.h"
#include <IGESControl_Reader.hxx>


FileReader::FileReader()
{

}

Standard_Integer FileReader::ReadIGES(const Standard_CString& aFileName,Handle(TopTools_HSequenceOfShape)& aHSequenceOfShape)
{
    IGESControl_Reader Reader;
    Standard_Integer status = Reader.ReadFile(aFileName);
    if (status != IFSelect_RetDone)
    {
        return status;
    }
    Reader.TransferRoots();
    TopoDS_Shape aShape = Reader.OneShape();
    aHSequenceOfShape->Append(aShape);
    return status;
}
