#include "include/utils.h"
#include "include/rayTracer.h"

class MyVRApp : public VRApp {
public:
	MyVRApp(int argc, char** argv) : VRApp(argc, argv) {
        rayTracer_ = new RayTracer();
    }

	void onVREvent(const VREvent &event) {
        //event.print();
        if (rayTracer_->ParseEvent(event))
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
            rayTracer_->Render(renderState);
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
