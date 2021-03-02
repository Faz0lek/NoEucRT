#include "NoEucEngine.hpp"
#include <iostream>
#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include "GouraudShader.hpp"

NoEucEngine::NoEucEngine() :
	width(800),
	height(600),
	window(sf::VideoMode(width, height, 24), "Non-Euclidean Ray Tracer", sf::Style::None),
	scene(),
	renderer(width, height, scene.mainCamera.fov),
	texture(),
	renderedImage(),
	movementClock()
{
	texture.create(width, height);
	renderedImage.setTexture(texture);

	fpsFont.loadFromFile("arial.ttf");
	fpsText.setFont(fpsFont);
	fpsText.setCharacterSize(18);
	fpsText.setFillColor(sf::Color::Red);

	window.setMouseCursorVisible(false);
	sf::Mouse::setPosition(sf::Vector2i(width * 0.5, height * 0.5), window);

	activeShader = new GouraudShader();
}

int NoEucEngine::run()
{
	// Model preprocessing
	for (auto& model : scene.objects)
	{
		// Transform every vertex to world space
		for (auto& vertex : model.vertices)
		{
			vertex = model.toWorld * vertex;
		}

		// Primitive assembly
		model.assembleTriangles();

		// Construst BVH on model
		model.constructBVH();
	}

	// Ray preprocessing, this function precomputes all ray directions in camera space.
	// Rays are then stored sequentially in a vector for simd support.
	renderer.precomputeRays();

	sf::Clock fpsClock;

	while (window.isOpen())
	{
		// Process events
		handleEvents();
		handleMovement();
		fpsText.setString(std::to_string(1 / fpsClock.restart().asSeconds()));

		// Render image
		renderer.render(scene, *activeShader, texture);

		// Display image
		window.clear();
		window.draw(renderedImage);
		window.draw(fpsText);
		window.display();
	}

	return 0;
}

void NoEucEngine::handleEvents()
{
	sf::Event event;
	while (window.pollEvent(event))
	{
		switch (event.type)
		{
		case sf::Event::Closed:
			window.close();
			break;

		case sf::Event::KeyPressed:
			if (event.key.code == sf::Keyboard::Escape)
			{
				window.close();
			}
			break;
		}
	}
}

void NoEucEngine::handleMovement()
{
	sf::Time elapsedTime = movementClock.restart();

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
	{
		scene.mainCamera.toWorld = glm::translate(scene.mainCamera.toWorld, { 0, 0, -elapsedTime.asSeconds() * scene.mainCamera.speed });
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
	{
		scene.mainCamera.toWorld = glm::translate(scene.mainCamera.toWorld, { -elapsedTime.asSeconds() * scene.mainCamera.speed, 0, 0 });
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
	{
		scene.mainCamera.toWorld = glm::translate(scene.mainCamera.toWorld, { 0, 0, elapsedTime.asSeconds() * scene.mainCamera.speed });
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
	{
		scene.mainCamera.toWorld = glm::translate(scene.mainCamera.toWorld, { elapsedTime.asSeconds() * scene.mainCamera.speed, 0, 0 });
	}

	sf::Vector2i mousePosition = sf::Mouse::getPosition(window);

	float deltaX = mousePosition.x - width * 0.5;
	float deltaY = mousePosition.y - height * 0.5;

	scene.mainCamera.toWorld = glm::rotate(scene.mainCamera.toWorld, -deltaY * 0.01f, { 1, 0, 0 });
	scene.mainCamera.toWorld = glm::rotate(scene.mainCamera.toWorld, -deltaX * 0.01f, { 0, 1, 0 });

	sf::Mouse::setPosition(sf::Vector2i( width * 0.5, height * 0.5), window);
}
