#pragma once

#include "Drift/Engine/Camera/ICamera.h"
#include "Drift/Engine/Input/InputTypes.h"
#include <memory>

namespace Drift::Engine::Camera {

    // Interface abstrata para controladores de câmera
    class ICameraController {
    public:
        virtual ~ICameraController() = default;
        
        // Atualiza o controlador baseado no input
        virtual void Update(float deltaTime, const Input::InputFrame& input) = 0;
        
        // Acesso à câmera controlada
        virtual ICamera* GetCamera() = 0;
        virtual const ICamera* GetCamera() const = 0;
        
        // Controle de ativação
        virtual void SetEnabled(bool enabled) { _enabled = enabled; }
        virtual bool IsEnabled() const { return _enabled; }
        
    protected:
        bool _enabled = true;
    };

} // namespace Drift::Engine::Camera 