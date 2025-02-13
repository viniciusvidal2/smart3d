#pragma once
#include <vtkRenderingAnnotationModule.h> // For export macro
#include <vtkActor2D.h>

class vtkPolyData;
class vtkTextActor;
class vtkTextProperty;
class vtkPolyDataMapper;
class vtkPolyDataMapper2D;
class vtkGlyph3D;
class vtkAppendPolyData;
class vtkActor;
class vtkAlgorithmOutput;
class captionActor2DConnection;

class captionActor2D : public vtkActor2D
{
public:
  vtkTypeMacro(captionActor2D, vtkActor2D);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  static captionActor2D *New();

  //@{
  /**
  * Define the text to be placed in the caption. The text can be multiple
  * lines (separated by "\n").
  */
  virtual void SetCaption(const char* caption);
  virtual char* GetCaption();
  //@}

  //@{
  /**
  * Set/Get the attachment point for the caption. By default, the attachment
  * point is defined in world coordinates, but this can be changed using
  * vtkCoordinate methods.
  */
  vtkWorldCoordinateMacro(AttachmentPoint);
  //@}

  //@{
  /**
  * Enable/disable the placement of a border around the text.
  */
  vtkSetMacro(Border, vtkTypeBool);
  vtkGetMacro(Border, vtkTypeBool);
  vtkBooleanMacro(Border, vtkTypeBool);
  //@}

  //@{
  /**
  * Enable/disable drawing a "line" from the caption to the
  * attachment point.
  */
  vtkSetMacro(Leader, vtkTypeBool);
  vtkGetMacro(Leader, vtkTypeBool);
  vtkBooleanMacro(Leader, vtkTypeBool);
  //@}

  //@{
  /**
  * Indicate whether the leader is 2D (no hidden line) or 3D (z-buffered).
  */
  vtkSetMacro(ThreeDimensionalLeader, vtkTypeBool);
  vtkGetMacro(ThreeDimensionalLeader, vtkTypeBool);
  vtkBooleanMacro(ThreeDimensionalLeader, vtkTypeBool);
  //@}

  //@{
  /**
  * Specify a glyph to be used as the leader "head". This could be something
  * like an arrow or sphere. If not specified, no glyph is drawn. Note that
  * the glyph is assumed to be aligned along the x-axis and is rotated about
  * the origin. SetLeaderGlyphData() directly uses the polydata without
  * setting a pipeline connection. SetLeaderGlyphConnection() sets up a
  * pipeline connection and causes an update to the input during render.
  */
  virtual void SetLeaderGlyphData(vtkPolyData*);
  virtual void SetLeaderGlyphConnection(vtkAlgorithmOutput*);
  virtual vtkPolyData* GetLeaderGlyph();
  //@}

  //@{
  /**
  * Specify the relative size of the leader head. This is expressed as a
  * fraction of the size (diagonal length) of the renderer. The leader
  * head is automatically scaled so that window resize, zooming or other
  * camera motion results in proportional changes in size to the leader
  * glyph.
  */
  vtkSetClampMacro(LeaderGlyphSize, double, 0.0, 0.1);
  vtkGetMacro(LeaderGlyphSize, double);
  //@}

  //@{
  /**
  * Specify the maximum size of the leader head (if any) in pixels. This
  * is used in conjunction with LeaderGlyphSize to cap the maximum size of
  * the LeaderGlyph.
  */
  vtkSetClampMacro(MaximumLeaderGlyphSize, int, 1, 1000);
  vtkGetMacro(MaximumLeaderGlyphSize, int);
  //@}

  //@{
  /**
  * Specify the minimum size of the leader head (if any) in pixels. This
  * is used in conjunction with LeaderGlyphSize to cap the minimum size of
  * the LeaderGlyph.
  */
  vtkSetClampMacro(MinimumLeaderGlyphSize, int, 1, 1000);
  vtkGetMacro(MinimumLeaderGlyphSize, int);
  //@}

  //@{
  /**
  * Set/Get the padding between the caption and the border. The value
  * is specified in pixels.
  */
  vtkSetClampMacro(Padding, int, 0, 50);
  vtkGetMacro(Padding, int);
  //@}

  //@{
  /**
  * Get the text actor used by the caption. This is useful if you want to control
  * justification and other characteristics of the text actor.
  */
  vtkGetObjectMacro(TextActor, vtkTextActor);
  //@}

  //@{
  /**
  * Set/Get the text property.
  */
  virtual void SetCaptionTextProperty(vtkTextProperty *p);
  vtkGetObjectMacro(CaptionTextProperty, vtkTextProperty);
  //@}

  /**
  * Shallow copy of this scaled text actor. Overloads the virtual
  * vtkProp method.
  */
  void ShallowCopy(vtkProp *prop) override;

  //@{
  /**
  * Enable/disable whether to attach the arrow only to the edge,
  * NOT the vertices of the caption border.
  */
  vtkSetMacro(AttachEdgeOnly, vtkTypeBool);
  vtkGetMacro(AttachEdgeOnly, vtkTypeBool);
  vtkBooleanMacro(AttachEdgeOnly, vtkTypeBool);
  //@}

  /**
  * WARNING: INTERNAL METHOD - NOT INTENDED FOR GENERAL USE
  * DO NOT USE THIS METHOD OUTSIDE OF THE RENDERING PROCESS.
  * Release any graphics resources that are being consumed by this actor.
  * The parameter window could be used to determine which graphic
  * resources to release.
  */
  void ReleaseGraphicsResources(vtkWindow *) override;

  //@{
  /**
  * WARNING: INTERNAL METHOD - NOT INTENDED FOR GENERAL USE
  * DO NOT USE THIS METHOD OUTSIDE OF THE RENDERING PROCESS.
  * Draw the legend box to the screen.
  */
  int RenderOpaqueGeometry(vtkViewport* viewport) override;
  int RenderTranslucentPolygonalGeometry(vtkViewport*) override { return 0; }
  int RenderOverlay(vtkViewport* viewport) override;
  //@}

  /**
  * Does this prop have some translucent polygonal geometry?
  */
  int HasTranslucentPolygonalGeometry() override;

protected:
  captionActor2D();
  ~captionActor2D() override;

  vtkCoordinate *AttachmentPointCoordinate;

  vtkTypeBool   Border;
  vtkTypeBool   Leader;
  vtkTypeBool   ThreeDimensionalLeader;
  double LeaderGlyphSize;
  int   MaximumLeaderGlyphSize;
  int   MinimumLeaderGlyphSize;

  int   Padding;
  vtkTypeBool   AttachEdgeOnly;


private:
  vtkTextActor        *TextActor;
  vtkTextProperty     *CaptionTextProperty;

  vtkPolyData         *BorderPolyData;
  vtkPolyDataMapper2D *BorderMapper;
  vtkActor2D          *BorderActor;

  vtkPolyData         *HeadPolyData;    // single attachment point for glyphing
  vtkGlyph3D          *HeadGlyph;       // for 3D leader
  vtkPolyData         *LeaderPolyData;  // line represents the leader
  vtkAppendPolyData   *AppendLeader;    // append head and leader

                                        // for 2D leader
  vtkCoordinate       *MapperCoordinate2D;
  vtkPolyDataMapper2D *LeaderMapper2D;
  vtkActor2D          *LeaderActor2D;

  // for 3D leader
  vtkPolyDataMapper   *LeaderMapper3D;
  vtkActor            *LeaderActor3D;

  captionActor2DConnection* LeaderGlyphConnectionHolder;

private:
  captionActor2D(const captionActor2D&) = delete;
  void operator=(const captionActor2D&) = delete;
};
