#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QOpenGLWidget>
#include <mpv/client.h>
#include <mpv/render_gl.h>

class VideoWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);
    ~VideoWidget();

signals:
    void durationChanged(double time);
    void positionChanged(double time);

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;

private slots:
    void handleMpvEvents();
    void handleMpvEvent(mpv_event *event);
    void maybeUpdate();

private:
    static void onMpvGLUpdate(void *ctx);

    mpv_handle *mpv;
    mpv_render_context *mpvGL;
};

#endif // VIDEOWIDGET_H
