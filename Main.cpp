#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <thread>

constexpr double PI = 3.14159265359;
constexpr int WINDOW_WIDTH = 850;
constexpr int WINDOW_HEIGHT = 480;

typedef sf::Vector2<double> Vertex; // also used as vector bc lazy
typedef std::vector<Vertex> Vertices;

double Deg2Rad(const double& deg)
{
    return deg * PI / 180;
}

void RotatePoint(Vertex& point, const Vertex& rotation_center, const double& deg)
{
    const double angle = Deg2Rad(deg);
    Vertex translatedPoint(point.x - rotation_center.x,point.y-rotation_center.y); //translation to 0,0
    point = { translatedPoint.x * cos(-angle) - translatedPoint.y * sin(-angle) + rotation_center.x,translatedPoint.x * sin(-angle) + translatedPoint.y * cos(-angle) + rotation_center.y };
}

namespace Collision //created so it can work with any convex polygon
{
    double Dot(const Vertex& v1, const Vertex& v2)
    {
        return v1.x * v2.x + v1.y * v2.y;
    }

    void Project(const Vertices& vertices, const Vertex& axis, double& max, double& min)
    {
        for (const Vertex& v : vertices)
        {
            double projection = Dot(v, axis);

            if (projection < min) min = projection;
            if (projection > max) max = projection;
        }
    }
    bool Intersection(const Vertices& verticesA, const Vertices& verticesB)//returns true if figures intersect
    {
        for (int i = 0; i < verticesA.size(); ++i)
        {
            Vertex v1= verticesA[i];
            Vertex v2= verticesA[(i+1) % verticesA.size()];

            Vertex edge = v2 - v1;
            Vertex axis(-edge.y, edge.x); //creating a normal

            double minA = DBL_MAX;
            double maxA = DBL_MIN;
            double minB = DBL_MAX;
            double maxB = DBL_MIN;
            
            Project(verticesA, axis, maxA, minA);//projecting 1st rectangle
            Project(verticesB, axis, maxB, minB);//projecting 2nd rectangle

            if (minA >= maxB || minB >= maxA) return false;

        }
        
        for (int i = 0; i < verticesB.size(); ++i) // doing the same for other rect
        {
            Vertex v1 = verticesB[i];
            Vertex v2 = verticesB[(i+1) % verticesB.size()];

            Vertex edge = v2 - v1;
            Vertex axis(-edge.y, edge.x); //creating a normal

            double minA = DBL_MAX;
            double maxA = DBL_MIN;
            double minB = DBL_MAX;
            double maxB = DBL_MIN;

            Project(verticesA, axis, maxA, minA);
            Project(verticesB, axis, maxB, minB);

            if (minA >= maxB || minB >= maxA) return false;

        }
        return true;
    }
};

int main()
{
    sf::RectangleShape rec_mov({ 100,150 });
    sf::RectangleShape rec_nmov({ 50,50 });
    rec_nmov.setFillColor(sf::Color::Cyan);
    rec_nmov.setOrigin(rec_nmov.getGlobalBounds().height / 2, rec_nmov.getGlobalBounds().width / 2);

    std::random_device dev;
    std::mt19937 mt(dev());
    std::uniform_int_distribution<> xDist(rec_nmov.getGlobalBounds().width/2.f, WINDOW_WIDTH - rec_nmov.getGlobalBounds().width / 2.f);
    std::uniform_int_distribution<> yDist(rec_nmov.getGlobalBounds().height/2.f, WINDOW_HEIGHT - rec_nmov.getGlobalBounds().height / 2.f);
    //rec_nmov.setPosition(xDist(mt), yDist(mt));
    rec_nmov.setPosition(WINDOW_WIDTH/2,WINDOW_HEIGHT/2);

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH,WINDOW_HEIGHT), "SAT TESTING");


    sf::Vector2f v(0.f,0.f);
    sf::Vector2f newPos = rec_mov.getPosition();
    auto timer_start = std::chrono::steady_clock().now();
    auto timer_end = timer_start;
    float elapsedTime = 0;
    float refreshRate = 100000;

    float movementSpeed = 0.02f;

    Vertex point = { rec_nmov.getGlobalBounds().left, rec_nmov.getGlobalBounds().top };

    double rotation = rec_nmov.getRotation();

    sf::Mouse mouse;
    Vertex mouseVertex(mouse.getPosition(window).x, mouse.getPosition(window).y);
    Vertices mouseVertices = {mouseVertex};

    Vertices a = {
                {rec_mov.getGlobalBounds().left, rec_mov.getGlobalBounds().top},
                {rec_mov.getGlobalBounds().left + rec_mov.getGlobalBounds().width, rec_mov.getGlobalBounds().top},
                {rec_mov.getGlobalBounds().left + rec_mov.getGlobalBounds().width, rec_mov.getGlobalBounds().top + rec_mov.getGlobalBounds().height},
                {rec_mov.getGlobalBounds().left, rec_mov.getGlobalBounds().top + rec_mov.getGlobalBounds().height}
    };

    Vertices b = {
        {rec_nmov.getGlobalBounds().left, rec_nmov.getGlobalBounds().top},
        {rec_nmov.getGlobalBounds().left + rec_nmov.getGlobalBounds().width, rec_nmov.getGlobalBounds().top},
        {rec_nmov.getGlobalBounds().left + rec_nmov.getGlobalBounds().width, rec_nmov.getGlobalBounds().top + rec_nmov.getGlobalBounds().height},
        {rec_nmov.getGlobalBounds().left, rec_nmov.getGlobalBounds().top + rec_nmov.getGlobalBounds().height}
    };


    while (window.isOpen())
    {
        timer_end = std::chrono::steady_clock().now();
        elapsedTime = std::chrono::duration_cast<std::chrono::nanoseconds>(timer_end - timer_start).count();
        timer_start = timer_end;
        

        sf::Event event;

        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if(event.type == sf::Event::KeyPressed)
                switch (event.key.code)
                {
                case sf::Keyboard::Right:
                    v.x = movementSpeed;
                    break;
                case sf::Keyboard::Left:
                    v.x = -movementSpeed;
                    break;
                case sf::Keyboard::Up:
                    v.y = -movementSpeed;
                    break;
                case sf::Keyboard::Down:
                    v.y = movementSpeed;
                    break;
                default:
                    break;
                }
            if (event.type == sf::Event::KeyReleased)
                switch (event.key.code)
                {
                case sf::Keyboard::Right:
                    v.x = 0;
                    break;
                case sf::Keyboard::Left:
                    v.x = 0;
                    break;
                case sf::Keyboard::Up:
                    v.y = 0;
                    break;
                case sf::Keyboard::Down:
                    v.y = 0;
                    break;
                default:
                    break;
                }
        }
        while (elapsedTime > refreshRate)
        {
            elapsedTime -= refreshRate;
            rec_nmov.rotate(0.005);

            newPos = rec_mov.getPosition() + v;
            if (!(newPos.x > WINDOW_WIDTH - rec_mov.getGlobalBounds().width || newPos.x < 0)) rec_mov.setPosition(rec_mov.getPosition().x + v.x, rec_mov.getPosition().y);
            if (!(newPos.y > WINDOW_HEIGHT - rec_mov.getGlobalBounds().height || newPos.y < 0)) rec_mov.setPosition(rec_mov.getPosition().x, rec_mov.getPosition().y + v.y);

            mouseVertex.x = mouse.getPosition(window).x;
            mouseVertex.y = mouse.getPosition(window).y;
            mouseVertices = {mouseVertex};

            a = {
                {rec_mov.getGlobalBounds().left, rec_mov.getGlobalBounds().top},
                {rec_mov.getGlobalBounds().left + rec_mov.getGlobalBounds().width, rec_mov.getGlobalBounds().top},
                {rec_mov.getGlobalBounds().left + rec_mov.getGlobalBounds().width, rec_mov.getGlobalBounds().top + rec_mov.getGlobalBounds().height},
                {rec_mov.getGlobalBounds().left, rec_mov.getGlobalBounds().top + rec_mov.getGlobalBounds().height}
            };

            for (Vertex& v : b)
            {
                RotatePoint(v, { rec_nmov.getPosition().x,rec_nmov.getPosition().y }, -0.005);//this lil mofo oh god it took me eternity to find that the hitbox is spinning in the opposite direction its literally 4am
            }

            if (Collision::Intersection(a, b) || Collision::Intersection(mouseVertices,b))
            {
                rec_nmov.setFillColor(sf::Color::Red);
            }
            else
            {
                rec_nmov.setFillColor(sf::Color::Cyan);

            }


            window.clear();
            window.draw(rec_mov);
            window.draw(rec_nmov);
        }


        window.display();
    }

    return 0;
}