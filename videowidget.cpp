#include <cstring>
#include <QMouseEvent>
#include <QOpenGLContext>
#include <QTimer>
#include "videowidget.h"

static const char cmdLoadFile[] = "loadfile";
static const char cmdStop[] = "stop";
static const char msgMpvCreateException[] = "could not create mpv context";
static const char msgMpvInitializeException[] = "could not initialize mpv context";
static const char msgRenderContextException[] = "failed to initialize mpv GL context";
static const char propDScale[] = "dscale";
static const char propDuration[] = "duration";
static const char propEofReached[] = "eof-reached";
static const char propHwdec[] = "hwdec";
static const char propKeepOpen[] = "keep-open";
static const char propPause[] = "pause";
static const char propTimePos[] = "time-pos";
static const char propVolume[] = "volume";
static const char value0[] = "0";
static const char value100[] = "100";
static const char valueAuto[] = "auto";
static const char valueNo[] = "no";
static const char valueYes[] = "yes";
static const char valueSpline36[] = "spline36";

static void mpvWakeUp(void *ctx)
{
    QMetaObject::invokeMethod((VideoWidget*)ctx, "handleMpvEvents",
                              Qt::QueuedConnection);
}

static void *getGlProcAddress(void *ctx, const char *name)
{
    Q_UNUSED(ctx);
    QOpenGLContext *glCtx = QOpenGLContext::currentContext();
    if (!glCtx)
        return nullptr;
    return reinterpret_cast<void*>(glCtx->getProcAddress(QByteArray(name)));
}

VideoWidget::VideoWidget(QWidget *parent) : QOpenGLWidget(parent)
{
    setCursor(Qt::PointingHandCursor);

    mpv = mpv_create();
    if (!mpv)
        throw std::runtime_error(msgMpvCreateException);
    if (mpv_initialize(mpv) < 0)
        throw std::runtime_error(msgMpvInitializeException);
    mpv_set_option_string(mpv, "vo", "libmpv");
    mpv_set_option_string(mpv, propHwdec, valueAuto);
    mpv_set_option_string(mpv, propKeepOpen, valueYes);
    mpv_set_option_string(mpv, propDScale, valueSpline36);
    mpv_observe_property(mpv, 0, propDuration, MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, propEofReached, MPV_FORMAT_FLAG);
    mpv_observe_property(mpv, 0, propPause, MPV_FORMAT_FLAG);
    mpv_observe_property(mpv, 0, propTimePos, MPV_FORMAT_DOUBLE);
    mpv_set_wakeup_callback(mpv, mpvWakeUp, this);
}

VideoWidget::~VideoWidget()
{
    makeCurrent();
    if (mpvGL) {
        mpv_render_context_free(mpvGL);
        mpvGL = nullptr;
    }
    if (mpv) {
        mpvCommand({cmdStop});
        mpv_terminate_destroy(mpv);
        mpv = nullptr;
    } else {
        abort();
    }
}

void VideoWidget::setSilentMode(bool silent)
{
    mpvSetProperty(propVolume, silent ? value0 : value100);
}

void VideoWidget::setEarlyStopMode(bool earlyStop)
{
    earlyStopMode = earlyStop;
}

void VideoWidget::play(QString url)
{
    if (!glInitialized) {
        pendingFileOpen = url;
        return;
    }
    mpvCommand({cmdLoadFile, url});
    mpvSetProperty(propPause, valueNo);
}

void VideoWidget::stop()
{
    mpvCommand({cmdStop});
    mpvCommand({cmdLoadFile, ""});
}

void VideoWidget::pauseResume()
{
    mpvSetProperty(propPause, !mpvPaused ? valueYes : valueNo);
}

void VideoWidget::initializeGL()
{
    mpv_opengl_init_params glInitParams {
        getGlProcAddress, nullptr, nullptr
    };
    mpv_render_param renderParams[] {
        { MPV_RENDER_PARAM_API_TYPE, const_cast<char*>(MPV_RENDER_API_TYPE_OPENGL) },
        { MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, (void*)(&glInitParams) },
        { MPV_RENDER_PARAM_INVALID, nullptr }
    };
    if (mpv_render_context_create(&mpvGL, mpv, renderParams) < 0)
        throw std::runtime_error(msgRenderContextException);
    mpv_render_context_set_update_callback(mpvGL, VideoWidget::onMpvGLUpdate,
                                           reinterpret_cast<void*>(this));

    glInitialized = true;
    if (!pendingFileOpen.isEmpty()) {
        QTimer::singleShot(100, this, [this] {
            play(pendingFileOpen);
            pendingFileOpen.clear();
        });
    }
}

void VideoWidget::paintGL()
{
    qreal scale = devicePixelRatioF();
    mpv_opengl_fbo mpvFbo {
        static_cast<int>(defaultFramebufferObject()),
        int(width()*scale), int(height()*scale), 0
    };
    int flipY = 1;

    mpv_render_param renderParams[] = {
        { MPV_RENDER_PARAM_OPENGL_FBO, &mpvFbo },
        { MPV_RENDER_PARAM_FLIP_Y, &flipY },
        { MPV_RENDER_PARAM_INVALID, nullptr }
    };

    mpv_render_context_render(mpvGL, renderParams);
}

void VideoWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        pauseResume();
}

void VideoWidget::handleMpvEvents()
{
    while (mpv) {
        mpv_event *event = mpv_wait_event(mpv, 0);
        if (event->event_id == MPV_EVENT_NONE) {
            break;
        }
        handleMpvEvent(event);
    }
}

void VideoWidget::handleMpvEvent(mpv_event *event)
{
    switch (event->event_id) {
    case MPV_EVENT_PROPERTY_CHANGE: {
        mpv_event_property *prop = (mpv_event_property*)event->data;
        if (!strcmp(prop->name, propTimePos)) {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                double time = *(double*)prop->data;
                emit positionChanged(time);
                if (earlyStopMode && time > 5.0) {
                    mpvSetProperty(propPause, valueYes);
                }
            }
        } else if (!strcmp(prop->name, propDuration)) {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                double time = *(double*)prop->data;
                emit durationChanged(time);
            }
        } else if (!strcmp(prop->name, propEofReached)) {
            if (prop->format == MPV_FORMAT_FLAG) {
                bool flag = *(bool*)prop->data;
                if (flag)
                    emit eofReached();
            }
        } else if (!strcmp(prop->name, propPause)) {
            if (prop->format == MPV_FORMAT_FLAG) {
                bool flag = *(bool*)prop->data;
                mpvPaused = flag;
            }
        }
        break;
    }
    default:
        ;
    }
}

void VideoWidget::maybeUpdate()
{
    if (window()->isMinimized()) {
        makeCurrent();
        paintGL();
        context()->swapBuffers(context()->surface());
        doneCurrent();
    } else {
        update();
    }
}

void VideoWidget::mpvCommand(const QStringList &params)
{
    int n = params.count();
    char *args[n+1];
    QList<QByteArray> data;
    for (int i = 0; i < n; i++) {
        data.append(params[i].toUtf8());
        args[i] = data[i].data();
    }
    args[n] = nullptr;
    mpv_command(mpv, (const char**)args);
}

void VideoWidget::mpvSetProperty(const QString &name, const QString &value)
{
    QByteArray cName = name.toUtf8();
    QByteArray cValue = value.toUtf8();
    mpv_set_property_string(mpv, cName.data(), cValue.data());
}

void VideoWidget::onMpvGLUpdate(void *ctx)
{
    QMetaObject::invokeMethod((VideoWidget*)ctx,
                              "maybeUpdate");
}
