#include "plugin.hpp"

//Create our shaders from two strings - the source code of the shaders
//Again, the & gives us the id of the newly created shaders back

/*static int CreateShader(const std::string& vertexShader, const std::string& fragmentShader) {
    
    
    
};*/

struct ModularForecast : Module
{
    enum ParamIds
    {
        NUM_PARAMS
    };
    enum InputIds
    {
        ENUMS(CH_INPUTS, 1),
        NUM_INPUTS
    };
    enum OutputIds
    {
        MIX_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };
    
    dsp::ClockDivider lightDivider;
    float snowVoltage = 0.0;
    

    ModularForecast()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        lightDivider.setDivision(512);
    }

    void process(const ProcessArgs &args) override
    {
        //The final mix - it is 16 because there are 16 channels for
        //polyphony. The for loop below is what involves the audio inputs of the module.
        float mix[16] = {};
        int maxChannels = 1;
        
        // Channels
        for (int i = 0; i < 1; i++) {
            int channels = 1;
            float in[16] = {};
            
            if (inputs[CH_INPUTS + i].isConnected()) {
            
                channels = inputs[CH_INPUTS + i].getChannels();
                maxChannels = std::max(maxChannels, channels);
                
                // Get input - could be from 16 channels, who knows
                inputs[CH_INPUTS + i].readVoltages(in);
                
                // Add to mix
                for (int c = 0; c < channels; c++) {
                    mix[c] += in[c];
                }
                
            }
            
            //Set the output
            if (outputs[MIX_OUTPUT].isConnected()) {
                //Set the mix output
                outputs[MIX_OUTPUT].setChannels(maxChannels);
                outputs[MIX_OUTPUT].writeVoltages(mix);
            }
            
            snowVoltage = std::sqrt(std::pow(inputs[CH_INPUTS + 0].getVoltage(), 2));
        }
    }
};

//Particle simulator code by Eduardo Soto
//GitHub Link: https://github.com/sotoea/2D-Particle-System

const int MaxParticles = 1000;
int currentParticles = 500;

double mouseX;
double mouseY;
double sizeChange = 1.0;
double speedChange = 15.0;
int range = 8000;
int pullPush;

typedef struct
{
    double XCoor;
    double YCoor;
    double dX;
    double dY;
    double Red;
    double Green;
    double Blue;
    double size;
    bool affected;
} PARTICLE;

PARTICLE Particles[MaxParticles];

void initParticles(void)
{
    int i;
    for (i = 0; i < currentParticles; i++)
    {
        Particles[i].XCoor = rand() % 600 * 1.0;
        Particles[i].YCoor = rand() % 600 * 1.0;
        Particles[i].Blue = rand() % 2;
        Particles[i].Red = rand() % 2;
        Particles[i].Green = rand() % 2;
        Particles[i].dX = (rand() % 10 * 0.006) * pow(-1.0, rand() % 2);
        Particles[i].dY = (rand() % 10 * 0.006) * pow(-1.0, rand() % 2);
        Particles[i].size = (rand() % 12 + 5);
        Particles[i].affected = false;
    }
}

struct ModularForecastDisplay : OpenGlWidget
{
    ModularForecast *module;
    ModularForecastDisplay() {}

    void displayParticles(void)
    {
        int i;
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.15f, 0.15f, 0.3f, 0.0f);
        glRasterPos2f(fbSize.x - 0.892 * (fbSize.x), fbSize.y - 0.008 * (fbSize.y));
        for (i = 0; i < currentParticles; i++)
        {

            Particles[i].XCoor += Particles[i].dX * speedChange * (1 + (3 * module->snowVoltage)); //Logic for moving the particle
            Particles[i].YCoor += Particles[i].dY * speedChange * (1 + (3 * module->snowVoltage));

            if ((Particles[i].XCoor <= 1) | (Particles[i].XCoor >= 599))
            { //Logic for bouncing off the wall
                Particles[i].dX = Particles[i].dX * (1);
            }
            if ((Particles[i].YCoor <= 1) | (Particles[i].YCoor >= 599))
            {
                Particles[i].dY = Particles[i].dY * (-1);
            }

            /*mouseX/Y is set to the mouse location in x and y and used here, it is then reversed to mouseX\Y = (-1, -1)
                for no interference, this is used when the LMB is pressed and the particle stops at the cursor instead of keep going*/
            if (Particles[i].XCoor + 1 >= mouseX & Particles[i].XCoor - 1 <= mouseX & Particles[i].YCoor + 1 >= mouseY & Particles[i].YCoor - 1 <= mouseY)
            {
                Particles[i].dX = 0;
                Particles[i].dY = 0;
            }

            for (int j = 0; j <= currentParticles; j++)
            {
                if ((Particles[i].YCoor > 601) || (Particles[i].YCoor < -1))
                {

                    Particles[i].YCoor = 599;
                }
                if (Particles[i].XCoor < 1)
                {

                    Particles[i].XCoor = 599;
                }
                else if (Particles[i].XCoor > 599)
                {
                    Particles[i].XCoor = 1;
                }
            }
            if (Particles[i].affected == true)
            {                 //If the particle is affected by pull or push, make it move properly
                double temp1; //temp variables for performing calculations
                double temp2;
                if (pullPush == 0)
                { //pullPush == 0 is pull, 1 is for push
                    int i;
                    for (i = 0; i < currentParticles; i++)
                    { //Calculate relative distance in both x and y between each particle and mouse
                        temp1 = Particles[i].XCoor - mouseX;
                        temp2 = (Particles[i].YCoor) - mouseY;

                        if (pow(temp1, 2) + pow(temp2, 2) <= range)
                        {                                      //Relate the distance in between with the range
                            Particles[i].dX = -temp1 * 0.0015; //Change direction of particle
                            Particles[i].dY = -temp2 * 0.0015;
                        }
                    }
                }

                if (pullPush == 1)
                {
                    int i;

                    for (i = 0; i < currentParticles; i++)
                    {
                        temp1 = Particles[i].XCoor - mouseX;
                        temp2 = Particles[i].YCoor - mouseY;

                        if (pow(temp1, 2) + pow(temp2, 2) <= range)
                        {
                            Particles[i].dX = temp1 * 0.0015;
                            Particles[i].dY = temp2 * 0.0015;
                        }
                    }
                }
            }
            glPointSize(Particles[i].size * sizeChange); //Set size
            glBegin(GL_POINTS);                          //Draw particle with openGL
            glColor3f(1.0, 1.0, 1.0);
            glVertex2f(Particles[i].XCoor, Particles[i].YCoor);
            glRasterPos2f(0.667 * (fbSize.x), 0.667 * (fbSize.y));
            glEnd();
            glColor3f(0.8, 1.0, 0.8);
        }
    }

    void drawFramebuffer() override
    {

        /*//Array to hold data inside the buffer (vertex positions)
           float positions[6] = {
              -0.5f, -0.5f,
               0.0f, 0.5f,
               0.5f, -0.5f
           };
           
           unsigned int buffer;
           //Generate a buffer with an id
           glGenBuffers(1, &buffer);
           
           //Bind the buffer - The buffer which is currently bound is going to be drawn
           glBindBuffer(GL_ARRAY_BUFFER, buffer);
           
           //Provide the buffer with data
           //size in bytes
           glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), positions, GL_STATIC_DRAW);
        
           //Illustrate the layout of the vertex buffer to OpenGL
           glEnableVertexAttribArray(0);
           glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);*/

        //Draw blue rectangle inside T.V. instead of default rainbow triangle
        //glViewport(0.0, 0.0, fbSize.x, fbSize.y);
        //glViewport(0.0, 0.0, fbSize.x, fbSize.y);
        displayParticles();
    }
};

struct ModularForecastWidget : ModuleWidget
{
    ModularForecastWidget(ModularForecast *module);
};

ModularForecastWidget::ModularForecastWidget(ModularForecast *module)
{
    initParticles();
    glOrtho(0, 600, 0, 600, -1, 1);
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ModularForecast.svg")));

    ModularForecastDisplay *display = new ModularForecastDisplay();
    display->module = module;
    display->box.pos = Vec(150, (RACK_GRID_HEIGHT / 8) - 10);
    display->box.size = Vec(box.size.x - 190, RACK_GRID_HEIGHT - 80);
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addInput(createInput<PJ301MPort>(Vec(18, 55), module, ModularForecast::CH_INPUTS + 0));
    //addInput(createInput<PJ301MPort>(Vec(18, 115), module, ModularForecast::CH_INPUTS + 1));
    //addInput(createInput<PJ301MPort>(Vec(18, 175), module, ModularForecast::CH_INPUTS + 2));
    //addInput(createInput<PJ301MPort>(Vec(18, 235), module, ModularForecast::CH_INPUTS + 3));
    addOutput(createOutput<PJ301MPort>(Vec(18, 300), module, ModularForecast::MIX_OUTPUT));

    addChild(display);
};

Model *modelModularForecast = createModel<ModularForecast, ModularForecastWidget>("ModularForecast");

