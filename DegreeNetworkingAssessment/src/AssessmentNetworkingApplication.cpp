#include "AssessmentNetworkingApplication.h"
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>

#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <BitStream.h>

#include "Gizmos.h"
#include "Camera.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

using glm::vec3;
using glm::vec4;

AssessmentNetworkingApplication::AssessmentNetworkingApplication() 
: m_camera(nullptr),
m_peerInterface(nullptr) {

}

AssessmentNetworkingApplication::~AssessmentNetworkingApplication() {

}

bool AssessmentNetworkingApplication::startup() {

	// setup the basic window
	createWindow("Client Application", 1280, 720);

	Gizmos::create();

	// set up basic camera
	m_camera = new Camera(glm::pi<float>() * 0.25f, 16 / 9.f, 0.1f, 1000.f);
	m_camera->setLookAtFrom(vec3(10, 10, 10), vec3(0));

	// start client connection
	m_peerInterface = RakNet::RakPeerInterface::GetInstance();
	
	RakNet::SocketDescriptor sd;
	m_peerInterface->Startup(1, &sd, 1);

	// request access to server
	std::string ipAddress = "";
	std::cout << "Connecting to server at: ";
	std::cin >> ipAddress;
	RakNet::ConnectionAttemptResult res = m_peerInterface->Connect(ipAddress.c_str(), SERVER_PORT, nullptr, 0);

	if (res != RakNet::CONNECTION_ATTEMPT_STARTED) {
		std::cout << "Unable to start connection, Error number: " << res << std::endl;
		return false;
	}

	return true;
}

void AssessmentNetworkingApplication::shutdown() {
	// delete our camera and cleanup gizmos
	delete m_camera;
	Gizmos::destroy();

	// destroy our window properly
	destroyWindow();
}

bool AssessmentNetworkingApplication::update(float deltaTime) {

	// close the application if the window closes
	if (glfwWindowShouldClose(m_window) ||
		glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		return false;

	// update camera
	m_camera->update(deltaTime);

	// handle network messages
	RakNet::Packet* packet;
	for (packet = m_peerInterface->Receive(); packet;
		m_peerInterface->DeallocatePacket(packet),
		packet = m_peerInterface->Receive()) {

		switch (packet->data[0]) {
		case ID_CONNECTION_REQUEST_ACCEPTED:
			std::cout << "Our connection request has been accepted." << std::endl;
			break;
		case ID_CONNECTION_ATTEMPT_FAILED:
			std::cout << "Our connection request failed!" << std::endl;
			break;
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			std::cout << "The server is full." << std::endl;
			break;
		case ID_DISCONNECTION_NOTIFICATION:
			std::cout << "We have been disconnected." << std::endl;
			break;
		case ID_CONNECTION_LOST:
			std::cout << "Connection lost." << std::endl;
			break;
		case ID_ENTITY_LIST: {
			// receive list of entities
			RakNet::BitStream stream(packet->data, packet->length, false);
			stream.IgnoreBytes(sizeof(RakNet::MessageID));
			unsigned int size = 0;
			stream.Read(size);

			// first time receiving entities
			if (m_aiEntities.size() == 0) {
				m_aiEntities.resize(size / sizeof(AIEntity));
			}
			if (m_localAiEntities.size() == 0) {
				m_localAiEntities.resize(size / sizeof(AIEntity));

				stream.Read((char*)m_localAiEntities.data(), size);
			}
			stream.Read((char*)m_aiEntities.data(), size);


			for (int i = 0; i < m_aiEntities.size(); ++i)
			{
				m_localAiEntities[i].velocity = m_aiEntities[i].velocity;
				float temp = sqrtf(pow(m_localAiEntities[i].position.x - m_aiEntities[i].position.x, 2) + pow(m_localAiEntities[i].position.y - m_aiEntities[i].position.y, 2));
				if (temp > 1.0f)
				{
					m_localAiEntities[i].displacedTick++;
				}
				else
				{
					m_localAiEntities[i].displacedTick = 0;
				}
				if (m_localAiEntities[i].displacedTick > 3 || m_aiEntities[i].teleported)
				{
					m_localAiEntities[i].position = m_aiEntities[i].position;
				}
			}
			break;
		}
		default:
			std::cout << "Received unhandled message." << std::endl;
			break;
		}
	}

	Gizmos::clear();

	// add a grid
	for (int i = 0; i < 21; ++i) {
		Gizmos::addLine(vec3(-10 + i, 0, 10), vec3(-10 + i, 0, -10),
						i == 10 ? vec4(1, 1, 1, 1) : vec4(0, 0, 0, 1));

		Gizmos::addLine(vec3(10, 0, -10 + i), vec3(-10, 0, -10 + i),
						i == 10 ? vec4(1, 1, 1, 1) : vec4(0, 0, 0, 1));
	}
	for (auto& ai : m_localAiEntities) {
		ai.position.x += ai.velocity.x * 0.0166666666666667f;
		ai.position.y += ai.velocity.y * 0.0166666666666667f;
	}
	return true;
}
void AssessmentNetworkingApplication::draw() {

	// clear the screen for this frame
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// draw entities
	for (auto& ai : m_localAiEntities) {
		vec3 p1 = vec3(ai.position.x + ai.velocity.x * 0.25f, 0, ai.position.y + ai.velocity.y * 0.25f);
		vec3 p2 = vec3(ai.position.x, 0, ai.position.y) - glm::cross(vec3(ai.velocity.x, 0, ai.velocity.y), vec3(0, 1, 0)) * 0.1f;
		vec3 p3 = vec3(ai.position.x, 0, ai.position.y) + glm::cross(vec3(ai.velocity.x, 0, ai.velocity.y), vec3(0, 1, 0)) * 0.1f;
		if (ai.id == 1)
			Gizmos::addTri(p1, p2, p3, glm::vec4(0, 1, 0, 1));
		else
			Gizmos::addTri(p1, p2, p3, glm::vec4(1, 0, 0, 1));
	}

	// display the 3D gizmos
	Gizmos::draw(m_camera->getProjectionView());
}