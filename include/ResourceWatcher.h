#pragma once

#include <External/FileWatcher/include/FileWatcher/FileWatcher.h>

// where UpdateListener is defined as such
class FileChangeListener: public FW::FileWatchListener
{
public:

  FileChangeListener()
  {
  }

  void handleFileAction(FW::WatchID watchid, const FW::String& dir, const FW::String& filename, FW::Action action);

};

/* Hold all the information about resources. */
class ResourceInfos
{
private:

  ResourceInfos()
  {
  }

public:
  
  /** Each resource corresponds to a filepath. Lookup table. */
  boost::unordered_map<std::string, boost::filesystem::path> resourceFiles;

  static ResourceInfos &get()
  {
    static ResourceInfos instance;
    return instance;
  }
  
};

class ResourceWatcher;

/** Contains information of a single entity. Can create entity from these information */
class EntityInfo
{
public:

  EntityInfo()
  {
  }

  EntityInfo(Ogre::Entity *e);

  Ogre::Entity *CreateEntity(Ogre::SceneManager *mgr);

  std::vector<std::string> materials;
  std::string meshName;
  Ogre::Entity *oldEntity;
};

class WatchedResource
{
private:
  friend class ResourceWatcher;
  friend class FileChangeListener;

  boost::filesystem::path filePath;
  FW::WatchID watchId;

  enum ComponentType
  {
    ComponentType_Texture,
    ComponentType_Material,
    ComponentType_FragmentProgram,
    ComponentType_VertexProgram
  };

  // components this file has. i.e .material file can have many material definitions
  std::vector<std::pair<ComponentType, Ogre::String>> components;

  void FillComponents();

  void ReloadMaterial(ResourceWatcher &watcher);
  void ReloadTexture();
  void ReloadMesh(ResourceWatcher &watcher);
  /* Find shaders defined in file and reload them*/
  void ReloadShader(ResourceWatcher &watcher);
  void ReloadProgram();

public:

  enum ResourceType
  {
    ResourceType_Unknown,
    ResourceType_Directory,
    ResourceType_Material,
    ResourceType_Shader,
    ResourceType_ShaderProgram,
    ResourceType_Texture,
    ResourceType_Mesh,
    ResourceType_Skeleton
  };

  void init(ResourceWatcher &watcher, FW::WatchID watchId, const boost::filesystem::path &filePath, ResourceType type);

  ResourceType resourceType;

  void Reload(ResourceWatcher &watcher);

};

class SubEntityInfo
{
public:

  Ogre::String materialName;
  Ogre::String vertexProgram;
  Ogre::String pixelProgram;

};

class ResourceWatcher
{
private:

  friend class WatchedResource;
  friend class FileChangeListener;

  ResourceWatcher()
      : fileWatcher(0),
        fileChangeListener(0),
        resourceInfos(&ResourceInfos::get())
  {
  }


  boost::unordered_map<boost::filesystem::path, WatchedResource> watchedFiles;

  /** Store material info of failed reload of sub entities*/
  boost::unordered_map<Ogre::SubEntity*, SubEntityInfo> failedSubEntities;

  // Affected entities
  std::vector<Ogre::Entity*> entities;

  std::vector<Ogre::SceneManager*> scenes; 
  
  // Queue resource directories in this, then add all at once.
  std::vector<std::pair<boost::filesystem::path, bool>> queuedResourceDirectories;

  // Used to call update only at fixed intervals
  Ogre::Timer timeSinceLastReload;

  FW::FileWatcher *fileWatcher;
  FileChangeListener *fileChangeListener;
  
  ResourceInfos *resourceInfos;

  WatchedResource::ResourceType DetermineResourceType(const boost::filesystem::path &pathToFile);

  WatchedResource * GetWatchedResource(const boost::filesystem::path &pathToFile);

  void FindSubEntitiesUsingMaterialFile(const boost::filesystem::path &filePath, std::vector<std::pair<Ogre::SubEntity*, Ogre::String>> &subEntitiesToBeReloaded);
  void FindRenderablesUsingMaterial(const std::string &materialName, std::set<Ogre::Renderable*>& subEntities);

  void SetSubEntitiesToBaseWhite(const std::string &materialName, std::vector<std::pair<Ogre::SubEntity*, Ogre::String>> &subEntitiesToBeReloaded);

  /* Reloads material and all associated program files and shaders. */
  void CompleteReloadOfMaterial(const std::string &materialName);
  void CompleteReloadOfMaterial(const std::string &materialName, std::set<Ogre::Renderable*> &renderables);

  void CompleteReloadOfShader(std::string shaderName);

  void ReloadGpuProgram(Ogre::GpuProgram *shader, std::set<std::pair<std::string, std::pair<int, int>> > &materials);

  bool CanShaderCompile(Ogre::GpuProgram *program);

public:

  boost::unordered_map<boost::filesystem::path, FW::WatchID> directories;

  enum ReloadType
  {
    ReloadType_Shader,
    ReloadType_Material,
    ReloadType_Program,
    ReloadType_Texture,
    ReloadType_Mesh,
    ReloadType_Skeleton
  };

  /*
   Callback when a resource is reloaded. Type and void* to resource.
   Ogre::Entity* if type is mesh
   Ogre::String* if type is material
   Ogre::GpuProgram* if type is shader
   Ogre::Texture* if type is texture
   Ogre::Skeleton* if type is skeleton
   */
  std::function<void(ReloadType, void*)> ResourceReloadedCallback;

  std::function<void(const std::string &shaderName)> ShaderCompileFailed;

  std::function<void(Ogre::Entity*, Ogre::Entity*)> EntityReloaded;

  enum ResourceModificationType
  {
    ResourceModified,
    ResourceAdded,
    ResourceRemoved
  };

  std::function<void(ResourceModificationType, const boost::filesystem::path &path)> ResourceModifiedActionCallback;

  /* Sometimes we receive same call more than once */
  struct WatchInfo
  {
    FW::WatchID watchid;
    FW::String filename;
    FW::Action action;
    bool operator==(const WatchInfo &other)
    {
      if(watchid != other.watchid)
        return false;
      if(action != other.action)
        return false;
      if(filename != other.filename)
        return false;
      return true;
    }
  };
  std::vector<WatchInfo> watchBuffer;

  std::set<WatchedResource*> resourcesToBeReloaded;

  const boost::filesystem::path *GetResourcePath(const Ogre::String &name);

  bool DoesAlreadyHaveWatchInfo(const WatchInfo &info)
  {
    for(auto i : watchBuffer)
    {
      if(i == info)
        return true;
    }
    return false;
  }
  void ClearWatchBuffer()
  {
    watchBuffer.clear();
  }
  
  /** Add a resource directory to queued, later call ParseQueuedResourceDirectories to parse all resources in it  */
  void AddResourceDirectoryToQueue(const boost::filesystem::path &p, bool isRecursive);
  
  /** Adds resource paths queued and clears the queue */
  void ParseQueuedResourceDirectories();

  void init();

  void destroy();

  static ResourceWatcher &get()
  {
    static ResourceWatcher instance;
    return instance;
  }

  static bool WaitTillExclusiveFileAccess(const boost::filesystem::path &filePath, int maxWaitTimeMilliSecs = 100);

  void update()
  {
    if(timeSinceLastReload.getMilliseconds() > 10)
    {
      if(!resourcesToBeReloaded.empty())
      {
        for(auto i : resourcesToBeReloaded)
        {
          try
          {
            boost::filesystem::ifstream file;
            file.open(i->filePath); // quick check if file can be opened
          }
          catch(...)
          {
            continue;
          }

          switch(i->resourceType)
          {
          case WatchedResource::ResourceType_Unknown:
            break;
          case WatchedResource::ResourceType_Directory:
            break;
          case WatchedResource::ResourceType_Material:
            for(auto &comp : i->components)
            {
              if(ResourceModifiedActionCallback)
                ResourceModifiedActionCallback(ResourceWatcher::ResourceRemoved, i->filePath);
            }
            break;
          case WatchedResource::ResourceType_Shader:
            if(ResourceModifiedActionCallback)
              ResourceModifiedActionCallback(ResourceWatcher::ResourceRemoved, i->filePath);
            break;
          case WatchedResource::ResourceType_ShaderProgram:
            for(auto &comp : i->components)
            {
              if(ResourceModifiedActionCallback)
                ResourceModifiedActionCallback(ResourceWatcher::ResourceRemoved, i->filePath);
            }
            break;
          case WatchedResource::ResourceType_Texture:
            if(ResourceModifiedActionCallback)
              ResourceModifiedActionCallback(ResourceWatcher::ResourceRemoved, i->filePath);
            break;
          case WatchedResource::ResourceType_Mesh:
            if(ResourceModifiedActionCallback)
              ResourceModifiedActionCallback(ResourceWatcher::ResourceRemoved, i->filePath);
            break;
          case WatchedResource::ResourceType_Skeleton:
            if(ResourceModifiedActionCallback)
              ResourceModifiedActionCallback(ResourceWatcher::ResourceRemoved, i->filePath);
            break;
          default:
            break;
          }

          i->Reload(*this);
          resourcesToBeReloaded.erase(i);
          break;
        }
      }
      watchBuffer.clear();
      fileWatcher->update();
    }
  }

  void OpenResource(const std::string &resourceName);
  void OpenShaderFile(const std::string &shaderFile);

  void ReloadMaterial(const Ogre::String &matName);

  bool AddResourceDirectory(const boost::filesystem::path &p, bool recursive = false, bool alsoParseScripts = false);
  void UpdateResourceDirectory(const boost::filesystem::path &p); // updates ogre for the modified directory
  void RemoveResourceDirectory(const boost::filesystem::path &p, bool removeChildren = false);

  void ParseScriptsInPath(const boost::filesystem::path &p);
  void ParseFile(const boost::filesystem::path &p);

  void AddScene(Ogre::SceneManager *scene);
  void RemoveScene(Ogre::SceneManager *scene);

};
