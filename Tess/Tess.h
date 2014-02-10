#include "util.h"
#include "pipeline.h"
#include "camera.h"
#include "texture.h"
#include "lighting_technique.h"
#include "glut_backend.h"
#include "mesh.h"

#define WINDOW_WIDTH  1080
#define WINDOW_HEIGHT 720

class Tess : public ICallbacks
{
public:

    Tess()
    {
        m_pGameCamera = NULL;
        m_directionalLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_directionalLight.AmbientIntensity = 1.0f;
        m_directionalLight.DiffuseIntensity = 0.01f;        
        m_directionalLight.Direction = Vector3f(1.0f, -1.0, 0.0);
        
        m_persProjInfo.FOV = 75.0f;
        m_persProjInfo.Height = WINDOW_HEIGHT;
        m_persProjInfo.Width = WINDOW_WIDTH;
        m_persProjInfo.zNear = .01f;
        m_persProjInfo.zFar = 100.0f;  
        
        m_pDisplacementMap = NULL;
        m_dispFactor = 0.25f;
        m_isWireframe = false;
    }

    virtual ~Tess()
    {
        SAFE_DELETE(m_pGameCamera);
        SAFE_DELETE(m_pMesh);
        SAFE_DELETE(m_pDisplacementMap);
    }

    bool Init()
    {
        Vector3f Pos(0.0f, 1.0f, -5.0f);
        Vector3f Target(0.0f, -0.2f, 1.0f);
        Vector3f Up(0.0, 1.0f, 0.0f);       

        m_pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT, Pos, Target, Up);
      
        if (!m_lightingEffect.Init()) {
            printf("Error initializing the lighting technique\n");
            return false;
        }
               
        GLint MaxPatchVertices = 0;
        glGetIntegerv(GL_MAX_PATCH_VERTICES, &MaxPatchVertices);
        printf("Max supported patch vertices %d\n", MaxPatchVertices);
        //glPatchParameteri(GL_PATCH_VERTICES, 3);
     
        glActiveTexture(GL_TEXTURE4);
        m_pDisplacementMap = new Texture(GL_TEXTURE_2D, "heightmap.jpg");
        
        if (!m_pDisplacementMap->Load()) {
            return false;
        }
        
        m_pDisplacementMap->Bind(DISPLACEMENT_TEXTURE_UNIT);
                
        glActiveTexture(GL_TEXTURE0);
        m_pColorMap = new Texture(GL_TEXTURE_2D, "diffuse.jpg");
        
        if (!m_pColorMap->Load()) {
            return false;
        }
        
        m_pColorMap->Bind(COLOR_TEXTURE_UNIT);

        m_lightingEffect.Enable();
                GLExitIfError();
        m_lightingEffect.SetColorTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
        m_lightingEffect.SetDisplacementMapTextureUnit(DISPLACEMENT_TEXTURE_UNIT_INDEX);
                GLExitIfError();
        m_lightingEffect.SetDirectionalLight(m_directionalLight);
                       GLExitIfError();
        m_lightingEffect.SetDispFactor(m_dispFactor);                        
        m_pMesh = new Mesh();

        return m_pMesh->LoadMesh("quad2.obj");
    }

    void Run()
    {
        GLUTBackendRun(this);
    }

    virtual void RenderSceneCB()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        m_pGameCamera->OnRender();        

        Pipeline p;
        p.Scale(2.0f, 2.0f, 2.0f);               
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        p.SetPerspectiveProj(m_persProjInfo);
        
        // render the objects as usual
        //m_lightingEffect.Enable();
        GLExitIfError();       
        m_lightingEffect.SetEyeWorldPos(m_pGameCamera->GetPos());        
        m_lightingEffect.SetVP(p.GetVPTrans());
        m_lightingEffect.SetWorldMatrix(p.GetWorldTrans());                
        m_lightingEffect.SetDispFactor(m_dispFactor);
        GLExitIfError();       
        m_pMesh->Render(NULL);
        GLExitIfError();
        glutSwapBuffers();
    }

       
    virtual void IdleCB()
    {
        RenderSceneCB();
    }

    virtual void SpecialKeyboardCB(int Key, int x, int y)
    {
        m_pGameCamera->OnKeyboard(Key);
    }


    virtual void KeyboardCB(unsigned char Key, int x, int y)
    {
        switch (Key) {
            case 27:
            case 'q':
                glutLeaveMainLoop();
                break;

            case '+':
                m_dispFactor += 0.01f;
                break;
                
            case '-':
                if (m_dispFactor >= 0.01f) {
                    m_dispFactor -= 0.01f;                    
                }
                break;
                
            case 'z':
                m_isWireframe = !m_isWireframe;
                
                if (m_isWireframe) {
                    glPolygonMode(GL_FRONT, GL_LINE);
                }
                else {
                    glPolygonMode(GL_FRONT, GL_FILL);
                }                    
        }
    }


    virtual void PassiveMouseCB(int x, int y)
    {
        m_pGameCamera->OnMouse(x, y);
    }
    
    
    virtual void MouseCB(int Button, int State, int x, int y)
    {
    }

private:

    LightingTechnique m_lightingEffect;
    Camera* m_pGameCamera;
    DirectionalLight m_directionalLight;
    Mesh* m_pMesh;
    PersProjInfo m_persProjInfo;
    Texture* m_pDisplacementMap;
    Texture* m_pColorMap;
    float m_dispFactor;
    bool m_isWireframe;
};
