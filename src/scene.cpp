#include "scene.h"

QList<Scene> Scene::scenes; // List of scenes from the vortex DB

Vortex::Vortex()
{
    id = 0;
    destName = QString();
    destPos = UVector();
}

Scene::Scene(QString sceneName)
{
    name = sceneName.toLower();
    vortexes = QList<Vortex>();
}

Scene* findScene(QString sceneName)
{
    for (int i=0; i<Scene::scenes.size(); i++)
        if (Scene::scenes[i].name.toLower() == sceneName.toLower())
            return &Scene::scenes[i];

    return new Scene("");
}

Vortex findVortex(QString sceneName, quint8 id)
{
    Scene scene(sceneName);
    for (int i=0; i<Scene::scenes.size(); i++)
        if (Scene::scenes[i].name .toLower()== sceneName.toLower())
            scene = Scene::scenes[i];

    for (int i=0; i<scene.vortexes.size(); i++)
        if (scene.vortexes[i].id == id)
            return scene.vortexes[i];

    return Vortex();
}

Vortex findVortex(Scene* scene, quint8 id)
{
    for (int i=0; i<scene->vortexes.size(); i++)
        if (scene->vortexes[i].id == id)
            return scene->vortexes[i];

    return Vortex();
}
