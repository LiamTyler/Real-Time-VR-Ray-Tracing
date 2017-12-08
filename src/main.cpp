#include "include/utils.h"
#include "include/rayTracer.h"

using namespace MinVR;

class MyVRApp : public VRApp {
    public:
        MyVRApp(int argc, char** argv) : VRApp(argc, argv) {
            rayTracer_ = new RayTracer();
        }

        void onVREvent(const VREvent &event) {
            // event.print();
            Event e = NO_EVENT;
            if (event.getName() == "FrameStart") {
                rayTracer_->setCurrentTime(event.getDataAsFloat("ElapsedSeconds"));
            } else {
                string name = event.getName();
                bool down = name[name.find('_') + 1] == 'D';
                if (down) {
                    if (name == "KbdEsc_Down") {
                        shutdown();
                    } else if (name == "KbdW_Down") {
                        e = L_BACKWARDS_DOWN;
                    } else if (name == "KbdS_Down") {
                        e = L_FORWARDS_DOWN;
                    } else if (name == "KbdA_Down") {
                        e = L_LEFT_DOWN;
                    } else if (name == "KbdD_Down") {
                        e = L_RIGHT_DOWN;
                    } else if (name == "KbdLeft_Down") {
                        e = R_LEFT_DOWN;
                    } else if (name == "KbdRight_Down") {
                        e = R_RIGHT_DOWN;
                    } else if (name == "KbdUp_Down") {
                        e = R_FORWARDS_DOWN;
                    } else if (name == "KbdDown_Down") {
                        e = R_BACKWARDS_DOWN;
                    }
                } else {
                    if (name == "KbdW_Up" || name == "KbdS_Up") {
                        e = L_FORWARDS_UP;
                    } else if (name == "KbdA_Up" || name == "KbdD_Up") {
                        e = L_LEFT_UP;
                    } else if (name == "KbdLeft_Up" || name == "KbdRight_Up") {
                        e = R_RIGHT_UP;
                    } else if (name == "KbdUp_Up" || name == "KbdDown_Up") {
                        e = R_FORWARDS_UP;
                    }
                }
            }
            if (rayTracer_->ParseEvent(e))
                shutdown();
        }

        void onVRRenderGraphicsContext(const VRGraphicsState &renderState) {
            if (renderState.isInitialRenderCall()) {
                rayTracer_->SetUp();
            }

            if (!isRunning()) {
                delete rayTracer_;
                return;
            }
        }

        void onVRRenderGraphics(const VRGraphicsState &renderState) {
            if (isRunning()) {
                const float* v = renderState.getViewMatrix();
                const float* p = renderState.getProjectionMatrix();

                mat4 view = glm::make_mat4(v);
                mat4 proj = glm::make_mat4(p);

                rayTracer_->Render(view, proj);
            }
        }

    private:
        RayTracer* rayTracer_;
};

/// Main method which creates and calls application
int main(int argc, char **argv) {
    MyVRApp app(argc, argv);
    app.run();
    return 0;
}
