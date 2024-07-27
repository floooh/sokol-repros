// reproducer for https://issues.chromium.org/issues/355605685

#define SOKOL_IMPL
#define SOKOL_GLES3
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_log.h"
#include "sokol_glue.h"

static struct {
    struct {
        sg_pass_action pass_action;
        sg_image img;
        sg_sampler smp;
        sg_attachments atts;
    } offscreen;
    struct {
        sg_pipeline pip;
    } display;
} state;

void init(void) {
    sg_setup(&(sg_desc){
        .environment = sglue_environment(),
        .logger.func = slog_func,
    });

    // 256x256 render target texture for offscreen rendering
    state.offscreen.img = sg_make_image(&(sg_image_desc){
        .render_target = true,
        .width = 256,
        .height = 256,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
    });

    // a sampler for offscreen rendering
    state.offscreen.smp = sg_make_sampler(&(sg_sampler_desc){
        .min_filter = SG_FILTER_NEAREST,
        .mag_filter = SG_FILTER_NEAREST,
    });

    // an attachments object with the above texture
    state.offscreen.atts = sg_make_attachments(&(sg_attachments_desc){
        .colors[0] = { .image = state.offscreen.img }
    });

    // a pass action for the offscreen pass
    state.offscreen.pass_action = (sg_pass_action){
        .colors[0] = { .load_action = SG_LOADACTION_CLEAR, .clear_value = { 1.0f, 0.0f, 0.0f, 1.0f } },
    };

    // a shader and pipeline for rendering a textured quad in the default pass
    sg_shader shd = sg_make_shader(&(sg_shader_desc){
        .vs = {
            .source =
                "#version 300 es\n"
                "const vec2 pos[4] = vec2[](vec2(0,0),vec2(1,0),vec2(0,1),vec2(1,1));\n"
                "out vec2 uv;\n"
                "void main() {\n"
                "  gl_Position = vec4(pos[gl_VertexID] - 0.5, 0.5, 1.0);\n"
                "  uv = pos[gl_VertexID];\n"
                "}\n",
        },
        .fs = {
            .source =
                "#version 300 es\n"
                "precision highp float;\n"
                "precision highp int;\n"
                "uniform highp sampler2D tex_smp;\n"
                "layout(location = 0) out highp vec4 frag_color;\n"
                "in highp vec2 uv;\n"
                "void main() {\n"
                "  frag_color = texture(tex_smp, uv);\n"
                "}\n",
            .images[0] = { .used = true },
            .samplers[0] = { .used = true },
            .image_sampler_pairs[0] = { .used = true, .image_slot = 0, .sampler_slot = 0, .glsl_name = "tex_smp" },
        }
    });
    state.display.pip = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = shd,
        .primitive_type = SG_PRIMITIVETYPE_TRIANGLE_STRIP,
    });
}

void frame(void) {

    // offscreen pass: cycle clear color
    float g = state.offscreen.pass_action.colors[0].clear_value.g + 0.01f;
    state.offscreen.pass_action.colors[0].clear_value.g = (g > 1.0f) ? 0.0f : g;
    sg_begin_pass(&(sg_pass){
        .action = state.offscreen.pass_action,
        .attachments = state.offscreen.atts,
    });
    sg_end_pass();

    // default pass: render a rectangle with the offscreen-rendered texture
    sg_begin_pass(&(sg_pass){
        .swapchain = sglue_swapchain(),
    });
    sg_apply_pipeline(state.display.pip);
    sg_apply_bindings(&(sg_bindings){
        .fs.images[0] = state.offscreen.img,
        .fs.samplers[0] = state.offscreen.smp,
    });
    sg_draw(0, 4, 1);
    sg_end_pass();
    sg_commit();
}

void cleanup(void) {
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .width = 640,
        .height = 480,
        .window_title = "chrome127-offscreen",
        .logger.func = slog_func,
    };
}