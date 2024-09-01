[CCode (cheader_filename = "vala/vala.h")]
namespace Basket {
    [CCode (cname = "Color", has_type_id = false)]
    public struct Color {
        public uint8 a;
        public uint8 b;
        public uint8 g;
        public uint8 r;
        public uint32 full;
    }

    [CCode (cname = "Vertex", has_type_id = false)]
    public struct Vertex {
        public float position[3];
        public float uv[2];
        public Color color;
    }

    [CCode (cname = "Triangle", has_type_id = false)]
    public struct Triangle {
        public Vertex a;
        public Vertex b;
        public Vertex c;
    }

    [CCode (cname = "Box", has_type_id = false)]
    public struct Box {
        public float min[3];
        public float max[3];
    }

    [CCode (cname = "Range", has_type_id = false)]
    public struct Range {
        public uint32 offset;
        public uint32 length;
    }

    [CCode (cname = "err_fatal")]
    public void fatal(string title, string format, ...);

    namespace Filesystem {
        [CCode (cname = "basket_filesystem_read")]
        public GLib.Bytes? read(string name);
    }

    namespace Save {
        [CCode (cname = "sav_identity")]
        public bool identity(string identity);

        [CCode (cname = "basket_save_store")]
        public bool store(GLib.Bytes data);

        [CCode (cname = "basket_save_retrieve")]
        public GLib.Bytes? retrieve();
    }

    namespace Model {
        [CCode (cname = "MeshSlice", has_type_id = false)]
        public struct MeshSlice {
            public Vertex[] data;
            public uint32 length;
            public Box box;
        }

        [CCode (cname = "SubMesh", has_type_id = false)]
        public struct SubMesh {
            public string name;
            public Range range;
        }

        [CCode (cname = "Model", has_type_id = false, destroy_function = "mod_free")]
        public class Model {
            public MeshSlice mesh;
            public SubMesh[] submeshes;
            public uint32 submesh_amount;
            public string extra;

            [CCode (cname = "mod_init")]
            public static int init(out Model model, string data);

            [CCode (cname = "mod_free")]
            public void free();
        }
    }

    namespace Audio {
        [CCode (cname = "Sound")]
        [SimpleType]
        public struct Sound : uint32 {}

        [CCode (cname = "Source")]
        [SimpleType]
        public struct Source : uint32 {}

        [CCode (cname = "aud_load_ogg")]
        public int load_ogg(out Sound sound, uint8[] mem, uint32 length, bool spatialize);

        [CCode (cname = "aud_init_source")]
        public int init_source(out Source source, Sound audio);

        [CCode (cname = "aud_free")]
        public void free(Source audio);

        [CCode (cname = "aud_play")]
        public void play(Source audio);

        [CCode (cname = "aud_set_position")]
        public void set_position(Source audio, float position[3]);

        [CCode (cname = "aud_set_velocity")]
        public void set_velocity(Source audio, float velocity[3]);

        [CCode (cname = "aud_set_paused")]
        public void set_paused(Source audio, bool paused);

        [CCode (cname = "aud_set_looping")]
        public void set_looping(Source audio, bool looping);

        [CCode (cname = "aud_set_pitch")]
        public void set_pitch(Source audio, float pitch);

        [CCode (cname = "aud_set_area")]
        public void set_area(Source audio, float distance);

        [CCode (cname = "aud_set_gain")]
        public void set_gain(Source audio, float gain);

        [CCode (cname = "aud_stop")]
        public void stop(Source audio);

        [CCode (cname = "aud_state")]
        public int state(Source audio);

        [CCode (cname = "aud_listener")]
        public void listener(float position[3]);

        [CCode (cname = "aud_orientation")]
        public void orientation(float towards[3], float up[3]);

        [CCode (cname = "aud_gain")]
        public void gain(float volume);

        [CCode (cname = "aud_pause")]
        public void pause(bool pause);
    }

    [CCode (cname = "Image", has_type_id = false, destroy_function = "img_free")]
    [Compact]
    public struct Image {
        public Color[] pixels;
        public uint16 w;
        public uint16 h;

        [CCode (cname = "basket_image_from_bytes")]
        public static Image? from_bytes(GLib.Bytes data);
    }

    namespace Math {
        [CCode (cname = "Frustum", has_type_id = false)]
        public struct Frustum {
            public float left[4];
            public float right[4];
            public float top[4];
            public float bottom[4];
            public float near[4];
            public float far[4];

            [CCode (cname = "frustum_from_mat4")]
            public void from_mat4(out Frustum f, float m[16]);

            [CCode (cname = "frustum_vs_aabb")]
            public bool vs_aabb(Frustum f, float min[3], float max[3]);

            [CCode (cname = "frustum_vs_sphere")]
            public bool vs_sphere(Frustum f, float pos[3], float radius);

            [CCode (cname = "frustum_vs_triangle")]
            public bool vs_triangle(Frustum f, float a[3], float b[3], float c[3]);
        }

        namespace Mat4 {
            [CCode (cname = "mat4_projection")]
            public void projection(out float out[16], float fovy, float aspect, float near, float far, bool infinite);

            [CCode (cname = "mat4_ortho")]
            public void ortho(out float out[16], float left, float right, float top, float bottom, float near, float far);

            [CCode (cname = "mat4_lookat")]
            public void lookat(out float out[16], float eye[3], float at[3], float up[3]);

            [CCode (cname = "mat4_mul")]
            public void mul(out float out[16], float a[16], float b[16]);

            [CCode (cname = "mat4_invert")]
            public void invert(out float out[16], float a[16]);

            [CCode (cname = "mat4_mulvec")]
            public void mulvec(out float out[3], float in[3], float m[16]);

            // Add other mat4 functions...
        }

        /*
        namespace Vec {
            [CCode (cname = "vec_dot")]
            public float vec_dot(float[] a, float[] b, int len);

            [CCode (cname = "vec_len")]
            public float vec_len(float[] in, int len);

            [CCode (cname = "vec_min")]
            public void vec_min(float[] out, float[] a, float[] b, int len);

            [CCode (cname = "vec_max")]
            public void vec_max(float[] out, float[] a, float[] b, int len);

        }

        // Add other vector functions...

        */
    }

    namespace Renderer {
        [CCode (cname = "Texture")]
        [SimpleType]
        public struct Texture : uint8 {}

        [CCode (cname = "TextureSlice", has_type_id = false)]
        public struct TextureSlice {
            public uint16 x;
            public uint16 y;
            public uint16 w;
            public uint16 h;
        }

        [CCode (cname = "Light", has_type_id = false)]
        public struct Light {
            public float position[3];
            public float color[3];
        }

        [CCode (cname = "RenderCall", has_type_id = false)]
        public struct RenderCall {
            public bool disable;
            public float model[16];
            public Color tint;
            public Model.MeshSlice mesh;
            public TextureSlice texture;
            public Range range;
        }

        [CCode (cname = "Quad", has_type_id = false)]
        public struct Quad {
            public float position[2];
            public float scale[2];
            public TextureSlice texture;
            public Color color;
        }

        [CCode (cname = "ren_log")]
        public void log(string str, ...);

        [CCode (cname = "ren_render")]
        public RenderCall* render(RenderCall call);

        [CCode (cname = "ren_light")]
        public void light(Light light);

        [CCode (cname = "ren_draw")]
        public RenderCall* draw(RenderCall call);

        [CCode (cname = "ren_quad")]
        public void quad(Quad quad);

        [CCode (cname = "ren_rect")]
        public void rect(int32 x, int32 y, uint32 w, uint32 h, Color color);

        [CCode (cname = "ren_camera")]
        public void camera(float from[3], float to[3], float up[3]);

        [CCode (cname = "ren_far")]
        public void far(float far, Color clear);

        [CCode (cname = "ren_ambient")]
        public void ambient(Color ambient);

        [CCode (cname = "ren_snapping")]
        public void snapping(uint8 snap);

        [CCode (cname = "ren_dithering")]
        public void dithering(bool dither);

        [CCode (cname = "ren_size")]
        public void size(out uint16 w, out uint16 h);

        [CCode (cname = "ren_mouse_position")]
        public void mouse_position(out int16 x, out int16 y);

        [CCode (cname = "ren_videomode")]
        public void videomode(uint16 w, uint16 h, bool force_ratio);

        [CCode (cname = "ren_tex_load")]
        public Texture tex_load(string data, uint32 length);

        [CCode (cname = "ren_tex_load_custom")]
        public Texture tex_load_custom(Image image);

        [CCode (cname = "ren_tex_free")]
        public void tex_free(Texture id);

        [CCode (cname = "ren_tex_bind")]
        public void tex_bind(Texture main, Texture lumos);
    }

    namespace Input {
        [CCode (cname = "RawBindings", has_type_id = false)]
        public struct RawBindings {
            public string[] up;
            public string[] down;
            public string[] left;
            public string[] right;
            public string[] action;
            public string[] quick;
            public string[] menu;
        }

        [CCode (cname = "Binding", has_type_id = false)]
        public struct Binding {
            public uint32[,] buttons;
        }

        [CCode (cname = "inp_bind")]
        public void bind(RawBindings controller);

        [CCode (cname = "inp_text")]
        public unowned string text();

        [CCode (cname = "inp_button")]
        public uint32 button(uint8 button);

        [CCode (cname = "inp_direction")]
        public bool direction(out float direction[2]);

        [CCode (cname = "inp_current")]
        public Binding current();

        [CCode (cname = "inp_from_code")]
        public unowned string from_code(uint32 code);

        [CCode (cname = "inp_mouse_position")]
        public void mouse_position(out uint16 x, out uint16 y);

        [CCode (cname = "inp_mouse_down")]
        public bool mouse_down(uint8 button);
    }

    namespace Engine {
        [CCode (cname = "BasketEngineApp", type_id = "basket_engine_app_get_type ()")]
        public class App : GLib.Object {
            [CCode (has_construct_function = false)]
            public App ();

            public bool headless;

            public virtual int frame (double alpha, double delta);
            public virtual int tick (double delta);
            public virtual int close ();

            public int run ();
        }

        [CCode (cname = "eng_close")]
        public void close();

        [CCode (cname = "eng_halt")]
        public void halt(string str, ...);

        [CCode (cname = "eng_tickrate")]
        public void tickrate(double hz);

        [CCode (cname = "eng_window_size")]
        public void window_size(out uint16 w, out uint16 h);

        [CCode (cname = "eng_is_focused")]
        public bool is_focused();

        [CCode (cname = "eng_set_debug")]
        public void set_debug(bool debug);

        [CCode (cname = "eng_is_debug")]
        public bool is_debug();

        [CCode (cname = "eng_set_title")]
        public void set_title(string title);
    }
}
