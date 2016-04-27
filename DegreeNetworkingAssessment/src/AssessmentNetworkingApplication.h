#pragma once

#include "BaseApplication.h"
#include "AIEntity.h"
#include <vector>

class Camera;

namespace RakNet {
	class RakPeerInterface;
}

class AssessmentNetworkingApplication : public BaseApplication {
public:

	AssessmentNetworkingApplication();
	virtual ~AssessmentNetworkingApplication();

	virtual bool startup();
	virtual void shutdown();

	virtual bool update(float deltaTime);

	virtual void draw();

private:
	float Lerp(float a_start, float a_end, float a_lerpAmount, float a_allowance);

	RakNet::RakPeerInterface*	m_peerInterface;

	Camera*						m_camera;

	std::vector<AIEntity>		m_aiEntities;
	std::vector<AIEntity>		m_localAiEntities;
	int m_packetsRecieved = 0;
};