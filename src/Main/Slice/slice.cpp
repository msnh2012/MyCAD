#include "slice.h"
#include "ui_slice.h"
#include <QDebug>
#include "ApplicationCommon.h"
#include "BRepAlgoAPI_Section.hxx"
#include "OpenGl_GraphicDriver.hxx"
#include "View.h"
#include <QElapsedTimer>
Slice::Slice(QWidget *parent):
    QDialog(parent),ui(new Ui::Slice)
{
    ui->setupUi(this);

    static Handle(OpenGl_GraphicDriver) aGraphicDriver;

    if (aGraphicDriver.IsNull())
    {
      Handle(Aspect_DisplayConnection) aDisplayConnection;
  #if !defined(_WIN32) && !defined(__WIN32__) && (!defined(__APPLE__) || defined(MACOSX_USE_GLX))
      aDisplayConnection = new Aspect_DisplayConnection (OSD_Environment ("DISPLAY").Value());
  #endif
      aGraphicDriver = new OpenGl_GraphicDriver (aDisplayConnection);
    }

    aViewer = new V3d_Viewer (aGraphicDriver);
    aViewer->SetDefaultViewSize (1000.0);
    aViewer->SetDefaultViewProj (V3d_XposYnegZpos);
    aViewer->SetComputedMode (Standard_True);
    aViewer->SetDefaultComputedMode (Standard_True);
    aViewer->SetDefaultLights();
    aViewer->SetLightOn();
    myContext = new AIS_InteractiveContext (aViewer);
    myContext->SetDisplayMode( 1, false);
    openGLWidget = new View(myContext,this);
    openGLWidget->setObjectName(QString::fromUtf8("openGLWidget"));

    ui->horizontalLayout_3->addWidget(openGLWidget);

    normal = gp_Dir(0,0,1);
    sd = Slice_Z;
}

Slice::~Slice()
{
    delete ui;
    delete openGLWidget;
}

void Slice::setData(Handle(TopTools_HSequenceOfShape) Sequence,DocumentCommon *adoc,SliceDirection sd)
{
    aSequence=Sequence;
    doc = adoc;
    ui->comboBox->clear();
    sliceindex.clear();
    for ( int i = 1; i <= aSequence->Length(); i++ )
    {
        TopoDS_Shape shape = aSequence->Value(i);
        Component * cur;
        for (int j=0;j<doc->myComponents.size();j++) {
            if(doc->myComponents[j]->getShape()==shape)
            {
                cur = doc->myComponents[j];
                sliceindex.append(j);
            }
        }
        ui->comboBox->addItem(cur->getName(),i-1);
    }
    if(sd != Slice_Z)
    {
        if(sd == Slice_X)
        {
            sd = Slice_X;
            normal = gp_Dir(1,0,0);
        }
        else{
            sd = Slice_Y;
            normal = gp_Dir(0,1,0);
        }
    }
}

void Slice::on_pushButton_clicked(bool checked)
{
    QElapsedTimer time;
    time.start();
    thickness = ui->thickSpinBox->value();

    for ( int i = 1; i <= aSequence->Length(); i++ )
    {
        TopoDS_Shape shape = aSequence->Value(i);
        Component * cur;
        for (int j=0;j<doc->myComponents.size();j++) {
            if(doc->myComponents[j]->getShape()==shape)
            {
                cur = doc->myComponents[j];
            }
        }
        Handle(TopTools_HSequenceOfShape) slicedata =cur->getSlice();
        slicedata->Clear();
        getPlaneSet(cur->getBox());
        int numberlayer=planeset.size();
        //#pragma omp parallel for num_threads(8)
        for (int l=0;l<numberlayer;l++) {
            gp_Pln plane(gp_Pnt(planeset[l].x(),planeset[l].y(),planeset[l].z()),normal);
            BRepAlgoAPI_Section S(shape,plane,Standard_False);
            S.ComputePCurveOn1(Standard_True);
            S.Approximation(Standard_True);
            S.Build();
            TopoDS_Shape R = S.Shape();
            slicedata->Append(R);
        }
    }
    int j = ui->comboBox->currentIndex();
    Handle(TopTools_HSequenceOfShape) slicedata =doc->myComponents[j]->getSlice();
    ui->layerCount->setText("1/"+QString::number(slicedata->Length()));
    ui->verticalSlider->setRange(1,slicedata->Length());
    on_comboBox_currentIndexChanged(ui->comboBox->currentIndex());
    float slicetime = time.elapsed()/1000.0f;
    doc->getApplication()->BEdt->append("slice time :"+QString::number(slicetime));
}

void Slice::getPlaneSet(const Bnd_Box &box)
{
    planeset.clear();
    double  	theXmin;
    double  	theYmin;
    double  	theZmin;
    double  	theXmax;
    double  	theYmax;
    double  	theZmax;
    double min,max;
    box.Get(theXmin,theYmin,theZmin,theXmax,theYmax,theZmax);
    //qDebug()<<theXmin<<" "<<theXmax<<" "<<theYmin<<" "<<theYmax<<" "<<theZmin<<" "<<theZmax;
    float initial_layer_thickness;
    int layernumber;
    switch (sd) {
    case Slice_X:
        min = theXmin;
        max = theXmax;
        initial_layer_thickness=min+thickness/2.0f;
        layernumber=int((max-initial_layer_thickness)/thickness)+1;
        planeset.resize(layernumber);
        planeset[0] = QVector3D(initial_layer_thickness,0,0);
        for (int i=1;i<layernumber;i++) {
            planeset[i]=QVector3D(initial_layer_thickness+thickness*i,0,0);
        }
        break;
    case Slice_Y:
        min = theYmin;
        max = theYmax;
        initial_layer_thickness=min+thickness/2.0f;
        layernumber=int((max-initial_layer_thickness)/thickness)+1;
        planeset.resize(layernumber);
        planeset[0] = QVector3D(0,initial_layer_thickness,0);
        for (int i=1;i<layernumber;i++) {
            planeset[i]=QVector3D(0,initial_layer_thickness+thickness*i,0);
        }
        break;
    case Slice_Z:
        min = theZmin;
        max = theZmax;
        initial_layer_thickness=min+thickness/2.0f;
        layernumber=int((max-initial_layer_thickness)/thickness)+1;
        planeset.resize(layernumber);
        planeset[0] = QVector3D(0,0,initial_layer_thickness);
        for (int i=1;i<layernumber;i++) {
            planeset[i]=QVector3D(0,0,initial_layer_thickness+thickness*i);
        }
        break;
    default:
        break;
    }

}

void Slice::on_checkBox_stateChanged(int arg1)
{
    if(sliceindex.empty())return;
    if(arg1 == 0)
    {
        myContext->EraseAll(Standard_True);
        int j = ui->comboBox->currentIndex();
        Handle(TopTools_HSequenceOfShape) slicedata =doc->myComponents[j]->getSlice();
        ui->layerCount->setText("1/"+QString::number(slicedata->Length()));
        myContext->Display( new AIS_Shape( slicedata->Value( 1 ) ), false );
        myContext->UpdateCurrentViewer();
    }
    else{
        myContext->EraseAll(Standard_True);
        int j = ui->comboBox->currentIndex();
        Handle(TopTools_HSequenceOfShape) slicedata =doc->myComponents[j]->getSlice();
        for ( int i = 1; i <= slicedata->Length(); i++ )
        {
          myContext->Display( new AIS_Shape( slicedata->Value( i ) ), false );
        }
        myContext->UpdateCurrentViewer();
    }
}

void Slice::on_verticalSlider_valueChanged(int value)
{
    if(ui->checkBox->isChecked()) return;
    myContext->EraseAll(Standard_True);
    int j = ui->comboBox->currentIndex();
    Handle(TopTools_HSequenceOfShape) slicedata =doc->myComponents[j]->getSlice();
    ui->layerCount->setText(QString::number(value)+"/"+QString::number(slicedata->Length()));
    myContext->Display( new AIS_Shape( slicedata->Value( value ) ), false );
    myContext->UpdateCurrentViewer();
}

void Slice::on_comboBox_currentIndexChanged(int index)
{
    if(sliceindex.empty())return;
    if(!ui->checkBox->isChecked())
    {
        myContext->EraseAll(Standard_True);
        Handle(TopTools_HSequenceOfShape) slicedata =doc->myComponents[index]->getSlice();
        ui->layerCount->setText("1/"+QString::number(slicedata->Length()));
        myContext->Display( new AIS_Shape( slicedata->Value( 1 ) ), false );
        myContext->UpdateCurrentViewer();
    }
    else{
        myContext->EraseAll(Standard_True);
        Handle(TopTools_HSequenceOfShape) slicedata =doc->myComponents[index]->getSlice();
        for ( int i = 1; i <= slicedata->Length(); i++ )
        {
          myContext->Display( new AIS_Shape( slicedata->Value( i ) ), false );
        }
        myContext->UpdateCurrentViewer();
    }
}
