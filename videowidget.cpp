#include <cstring>
#include <QOpenGLContext>
#include "videowidget.h"

static const char msgMpvCreateException[] = "could not create mpv context";
static const char msgMpvInitializeException[] = "could not initialize mpv context";
static const char msgRenderContextException[] = "failed to initialize mpv GL context";
static const char propDuration[] = "duration";
static const char propTimePos[] = "time-pos";
static const char propHwdec[] = "hwdec";
static const char valueAuto[] = "auto";

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
    mpv = mpv_create();
    if (!mpv)
        throw std::runtime_error(msgMpvCreateException);
    if (mpv_initialize(mpv) < 0)
        throw std::runtime_error(msgMpvInitializeException);

    mpv_set_option_string(mpv, propHwdec, valueAuto);
    mpv_observe_property(mpv, 0, propDuration, MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, propTimePos, MPV_FORMAT_DOUBLE);
    mpv_set_wakeup_callback(mpv, mpvWakeUp, this);
}

VideoWidget::~VideoWidget()
{
    mpv_destroy(mpv);
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
            }
        } else if (!strcmp(prop->name, propDuration)) {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                double time = *(double*)prop->data;
                emit durationChanged(time);
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

void VideoWidget::onMpvGLUpdate(void *ctx)
{
    QMetaObject::invokeMethod((VideoWidget*)ctx,
                              "maybeUpdate");
}
