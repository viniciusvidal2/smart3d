#include "OpenVRDeletePointsWidgetRepresentation.h"

#include <vtkObjectFactory.h>
#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkActor.h>
#include <vtkIdList.h>
#include <vtkPointLocator.h>
#include <vtkSelection.h>
#include <vtkSelectionNode.h>
#include <vtkExtractSelectedIds.h>
#include <vtkInformation.h>
#include <vtkDataSetMapper.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkEventData.h>

#include "Mesh.h"

vtkStandardNewMacro(OpenVRDeletePointsWidgetRepresentation);

//----------------------------------------------------------------------------
OpenVRDeletePointsWidgetRepresentation::OpenVRDeletePointsWidgetRepresentation()
{
  vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
  sphereSource->SetCenter(0.0, 0.0, 0.0);
  sphereSource->SetPhiResolution(50);
  sphereSource->SetThetaResolution(50);
  sphereSource->SetRadius(searchRadius);

  vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputConnection(sphereSource->GetOutputPort());
  actorSphere = vtkSmartPointer<vtkActor>::New();
  actorSphere->SetMapper(mapper);

  actorSphere->GetProperty()->SetOpacity(0.5);
  actorSphere->GetProperty()->SetColor(1.0, 1.0, 1.0);
  //mainIdList = vtkSmartPointer<vtkIdList>::New();

  this->InteractionState = OpenVRDeletePointsWidgetRepresentation::Outside;
}

//----------------------------------------------------------------------------
OpenVRDeletePointsWidgetRepresentation::~OpenVRDeletePointsWidgetRepresentation()
{
  actorSphere = NULL;
  actorPointsToDelete = NULL;
}

void OpenVRDeletePointsWidgetRepresentation::computePointsToDelete()
{
  if (pointLocator == NULL)
  {
    return;
  }
  vtkSmartPointer<vtkIdList> idList = vtkSmartPointer<vtkIdList>::New();
  pointLocator->FindPointsWithinRadius(searchRadius, actorSphere->GetPosition(), idList);
  if (idList->GetNumberOfIds() == 0)
  {
    actorPointsToDelete = NULL;
    selection = NULL;
    return;
  }
  if (selection == NULL)
  {
    selection = vtkSmartPointer<vtkSelection>::New();
  }
  vtkSmartPointer<vtkSelection> selectionTemp = vtkSmartPointer<vtkSelection>::New();
  selectionTemp->AddNode(createSelectionNode(idList));
  selection->Union(selectionTemp);
  createActorPointsToDelete();
}

vtkSmartPointer<vtkSelectionNode> OpenVRDeletePointsWidgetRepresentation::createSelectionNode(vtkSmartPointer<vtkIdList> idList)
{
  vtkIdType numIds = idList->GetNumberOfIds();
  vtkSmartPointer<vtkIdTypeArray> ids = vtkSmartPointer<vtkIdTypeArray>::New();
  ids->SetNumberOfComponents(1);
  ids->SetNumberOfValues(numIds);
  // Set values
  for (vtkIdType i = 0; i < numIds; i++)
  {
    ids->SetValue(i, idList->GetId(i));
  }
  vtkSmartPointer<vtkSelectionNode> selectionNode = vtkSmartPointer<vtkSelectionNode>::New();
  selectionNode->SetFieldType(vtkSelectionNode::POINT);
  selectionNode->SetContentType(vtkSelectionNode::INDICES);
  selectionNode->SetSelectionList(ids);
  return selectionNode;
}

void OpenVRDeletePointsWidgetRepresentation::createActorPointsToDelete()
{
  vtkSmartPointer<vtkExtractSelectedIds> extractSelectedIds = vtkSmartPointer<vtkExtractSelectedIds>::New();
  extractSelectedIds->SetInputData(0, mesh->getPolyData());
  extractSelectedIds->SetInputData(1, selection);
  extractSelectedIds->Update();

  vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New();
  mapper->SetInputConnection(extractSelectedIds->GetOutputPort());
  mapper->SetScalarVisibility(false);

  actorPointsToDelete = vtkSmartPointer<vtkActor>::New();
  actorPointsToDelete->SetMapper(mapper);
  actorPointsToDelete->GetProperty()->SetColor(1.0, 0.0, 0.0);
  actorPointsToDelete->GetProperty()->SetPointSize(2);
}

void OpenVRDeletePointsWidgetRepresentation::deletePoints()
{
  if (selection == NULL)
  {
    return;
  }
  size_t numNodes = selection->GetNumberOfNodes();
  for (size_t i = 0; i < numNodes; i++)
  {
    selection->GetNode(i)->GetProperties()->Set(vtkSelectionNode::INVERSE(), 1);
  }

  vtkSmartPointer<vtkExtractSelectedIds> extractSelectedIds = vtkSmartPointer<vtkExtractSelectedIds>::New();
  extractSelectedIds->SetInputData(0, mesh->getPolyData());
  extractSelectedIds->SetInputData(1, selection);
  extractSelectedIds->Update();

  vtkSmartPointer<vtkDataSetSurfaceFilter> surfaceFilter = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
  surfaceFilter->SetInputConnection(extractSelectedIds->GetOutputPort());
  surfaceFilter->Update();

  vtkSmartPointer<vtkPolyData> poly = surfaceFilter->GetOutput();
  mesh->updatePoints(poly);
  actorPointsToDelete = NULL;
  selection = NULL;
  if (poly->GetPoints() == NULL)
  {
    pointLocator = NULL;
  }
  else
  {
    pointLocator = vtkSmartPointer<vtkPointLocator>::New();
    pointLocator->SetDataSet(mesh->getPolyData());
    pointLocator->BuildLocator();
  }
}

void OpenVRDeletePointsWidgetRepresentation::setScale(double scale)
{
  actorSphere->SetScale(scale, scale, scale);
  searchRadius *= scale;
}

int OpenVRDeletePointsWidgetRepresentation::ComputeComplexInteractionState(
  vtkRenderWindowInteractor *,
  vtkAbstractWidget *,
  unsigned long, void *calldata, int)
{
  this->InteractionState = OpenVRDeletePointsWidgetRepresentation::Deleting;
  return this->InteractionState;
}

void OpenVRDeletePointsWidgetRepresentation::StartComplexInteraction(
  vtkRenderWindowInteractor *,
  vtkAbstractWidget *,
  unsigned long, void *calldata)
{
  this->actorSphere->GetProperty()->SetColor(1.0, 0.0, 0.0);
}

void OpenVRDeletePointsWidgetRepresentation::ComplexInteraction(
  vtkRenderWindowInteractor *,
  vtkAbstractWidget *,
  unsigned long, void *calldata)
{
  vtkEventData *edata = static_cast<vtkEventData *>(calldata);
  vtkEventDataDevice3D *edd = edata->GetAsEventDataDevice3D();
  if (edd)
  {
    double eventPos[3];
    edd->GetWorldPosition(eventPos);
    double eventDir[4];
    edd->GetWorldOrientation(eventDir);

    this->actorSphere->SetPosition(eventPos);
    if (mesh != NULL && this->InteractionState == OpenVRDeletePointsWidgetRepresentation::Deleting)
    {
      computePointsToDelete();
    }
    this->Modified();
  }
}

void OpenVRDeletePointsWidgetRepresentation::EndComplexInteraction(
  vtkRenderWindowInteractor *,
  vtkAbstractWidget *,
  unsigned long, void *)
{
  this->InteractionState = OpenVRDeletePointsWidgetRepresentation::Outside;
  this->actorSphere->GetProperty()->SetColor(1.0, 1.0, 1.0);
}


//----------------------------------------------------------------------------
void OpenVRDeletePointsWidgetRepresentation::ReleaseGraphicsResources(vtkWindow *w)
{
  this->actorSphere->ReleaseGraphicsResources(w);
  if (this->actorPointsToDelete != NULL)
  {
    this->actorPointsToDelete->ReleaseGraphicsResources(w);
  }
}

//----------------------------------------------------------------------------
int OpenVRDeletePointsWidgetRepresentation::RenderOpaqueGeometry(vtkViewport *v)
{
  if (!this->GetVisibility())
  {
    return 0;
  }
  int count = this->actorSphere->RenderOpaqueGeometry(v);
  if (this->actorPointsToDelete != NULL)
  {
    count += this->actorPointsToDelete->RenderOpaqueGeometry(v);
  }
  return count;
}

//-----------------------------------------------------------------------------
int OpenVRDeletePointsWidgetRepresentation::RenderTranslucentPolygonalGeometry(
  vtkViewport *v)
{
  if (!this->GetVisibility())
  {
    return 0;
  }
  int count = this->actorSphere->RenderTranslucentPolygonalGeometry(v);
  if (this->actorPointsToDelete != NULL)
  {
    count += this->actorPointsToDelete->RenderTranslucentPolygonalGeometry(v);
  }
  return count;
}

//-----------------------------------------------------------------------------
int OpenVRDeletePointsWidgetRepresentation::HasTranslucentPolygonalGeometry()
{
  if (!this->GetVisibility())
  {
    return 0;
  }

  int result = 0;

  result |= this->actorSphere->HasTranslucentPolygonalGeometry();
  if (this->actorPointsToDelete != NULL)
  {
    result |= this->actorPointsToDelete->HasTranslucentPolygonalGeometry();
  }
  return result;
}

void OpenVRDeletePointsWidgetRepresentation::setMesh(Mesh * mesh)
{
  this->mesh = mesh;
  pointLocator = vtkSmartPointer<vtkPointLocator>::New();
  pointLocator->SetDataSet(mesh->getPolyData());
  pointLocator->BuildLocator();
}

void OpenVRDeletePointsWidgetRepresentation::disable()
{
  actorPointsToDelete = NULL;
}

//----------------------------------------------------------------------------
void OpenVRDeletePointsWidgetRepresentation::PlaceWidget(double bds[6])
{
}

//----------------------------------------------------------------------------
void OpenVRDeletePointsWidgetRepresentation::BuildRepresentation()
{
}

//----------------------------------------------------------------------------
void OpenVRDeletePointsWidgetRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
