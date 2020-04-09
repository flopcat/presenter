#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QOpenGLWidget>
#include <mpv/client.h>
#include <mpv/render_gl.h>

class QMouseEvent;

class VideoWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);
    ~VideoWidget();

    void setSilentMode(bool silent);
    void setEarlyStopMode(bool earlyStop);

signals:
    void durationChanged(double time);
    void positionChanged(double time);
    void eofReached();

public slots:
    void play(QString url);
    void stop();
    void pauseResume();

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private slots:
    void handleMpvEvents();
    void handleMpvEvent(mpv_event *event);
    void maybeUpdate();

private:
    void mpvCommand(const QStringList &params);
    void mpvSetProperty(const QString &name, const QString &value);
    static void onMpvGLUpdate(void *ctx);

    mpv_handle *mpv = nullptr;
    mpv_render_context *mpvGL = nullptr;

    bool earlyStopMode = false;
    bool mpvPaused = false;
};

#endif // VIDEOWIDGET_H
