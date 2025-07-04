#include "PreCompile.h"

#include "ResourceWatcher.h"



void FileChangeListener::handleFileAction(FW::WatchID watchid, const FW::String& dir, const FW::String& filename, FW::Action action)
{
  using namespace boost::filesystem;
  ResourceWatcher::WatchInfo info;
  info.watchid = watchid;
  info.filename = filename;
  info.action = action;

  ResourceWatcher &resourceWatcher = ResourceWatcher::get();

  if(resourceWatcher.DoesAlreadyHaveWatchInfo(info))
    return;

  resourceWatcher.watchBuffer.push_back(info);

  switch(action)
  {
  case FW::Actions::Add:
    {
      boost::filesystem::path p(dir + L"/" + filename);
      if(is_regular_file(p))
      {
        // ignore if already in watch list
        if(resourceWatcher.GetWatchedResource(p))
          return;

        WatchedResource::ResourceType type = resourceWatcher.DetermineResourceType(p);
        if(type != WatchedResource::ResourceType_Unknown)
        {
          resourceWatcher.watchedFiles[p].init(ResourceWatcher::get(), watchid, p, type);

          switch(type)
          {
          case WatchedResource::ResourceType_Unknown:
            break;
          case WatchedResource::ResourceType_Directory:
            break;
          case WatchedResource::ResourceType_Material:
            if(resourceWatcher.ResourceModifiedActionCallback)
              resourceWatcher.ResourceModifiedActionCallback(ResourceWatcher::ResourceAdded, p);
            break;
          case WatchedResource::ResourceType_Shader:
            if(resourceWatcher.ResourceModifiedActionCallback)
              resourceWatcher.ResourceModifiedActionCallback(ResourceWatcher::ResourceAdded, p);
            break;
          case WatchedResource::ResourceType_ShaderProgram:
            if(resourceWatcher.ResourceModifiedActionCallback)
              resourceWatcher.ResourceModifiedActionCallback(ResourceWatcher::ResourceAdded, p);
            break;
          case WatchedResource::ResourceType_Texture:
            if(resourceWatcher.ResourceModifiedActionCallback)
              resourceWatcher.ResourceModifiedActionCallback(ResourceWatcher::ResourceAdded, p);
            break;
          case WatchedResource::ResourceType_Mesh:
            if(resourceWatcher.ResourceModifiedActionCallback)
              resourceWatcher.ResourceModifiedActionCallback(ResourceWatcher::ResourceAdded, p);
            break;
          case WatchedResource::ResourceType_Skeleton:
            if(resourceWatcher.ResourceModifiedActionCallback)
              resourceWatcher.ResourceModifiedActionCallback(ResourceWatcher::ResourceAdded, p);
            break;
          default:
            break;
          }

          resourceWatcher.UpdateResourceDirectory(p.parent_path());
        }
      }
      else
      {
        //TODO: handle added directories if directory mode is recursive 
      }
    }
    break;
  case FW::Actions::Delete:
    {
      boost::filesystem::path p(dir + L"/" + filename);
      WatchedResource *res = ResourceWatcher::get().GetWatchedResource(p);
      if(res)
      {

        switch(res->resourceType)
        {
        case WatchedResource::ResourceType_Unknown:
          break;
        case WatchedResource::ResourceType_Directory:
          break;
        case WatchedResource::ResourceType_Material:
          for(auto &comp : res->components)
          {
            if(resourceWatcher.ResourceModifiedActionCallback)
              resourceWatcher.ResourceModifiedActionCallback(ResourceWatcher::ResourceRemoved, p);
          }
          break;
        case WatchedResource::ResourceType_Shader:
          if(resourceWatcher.ResourceModifiedActionCallback)
            resourceWatcher.ResourceModifiedActionCallback(ResourceWatcher::ResourceRemoved, p);
          break;
        case WatchedResource::ResourceType_ShaderProgram:
          if(resourceWatcher.ResourceModifiedActionCallback)
            resourceWatcher.ResourceModifiedActionCallback(ResourceWatcher::ResourceRemoved, p);
          break;
        case WatchedResource::ResourceType_Texture:
          if(resourceWatcher.ResourceModifiedActionCallback)
            resourceWatcher.ResourceModifiedActionCallback(ResourceWatcher::ResourceRemoved, p);
          break;
        case WatchedResource::ResourceType_Mesh:
          if(resourceWatcher.ResourceModifiedActionCallback)
            resourceWatcher.ResourceModifiedActionCallback(ResourceWatcher::ResourceRemoved, p);
          break;
        case WatchedResource::ResourceType_Skeleton:
          if(resourceWatcher.ResourceModifiedActionCallback)
            resourceWatcher.ResourceModifiedActionCallback(ResourceWatcher::ResourceRemoved, p);
          break;
        default:
          break;
        }

        ResourceWatcher::get().watchedFiles.erase(p);
      }
    }
    //std::cout << "File (" << dir + "\\" + filename << ") Deleted! " << std::endl;
    break;
  case FW::Actions::Modified:
    if(filename != L"")
    {

      WatchedResource *res = 0;

      {
        boost::filesystem::path fileDir(dir);
        // if dir is c: d: e: etc...
        if(fileDir == fileDir.root_path())
          res = ResourceWatcher::get().GetWatchedResource(boost::filesystem::path(dir + filename));
        else
          res = ResourceWatcher::get().GetWatchedResource(boost::filesystem::path(dir + L"\\" + filename));
      }

      if(!res)
        return;

      try
      {
        boost::filesystem::ifstream file;
        file.open(res->filePath);
        if(!file.is_open())
        {
          ResourceWatcher::get().resourcesToBeReloaded.insert(res);
          return;
        }
      }
      catch(...)
      {
        // if can't open file add it queue, reload it later
        ResourceWatcher::get().resourcesToBeReloaded.insert(res);
        return;
      }

      if(res)
      {

        switch(res->resourceType)
        {
        case WatchedResource::ResourceType_Unknown:
          break;
        case WatchedResource::ResourceType_Directory:
          break;
        case WatchedResource::ResourceType_Material:
          if(resourceWatcher.ResourceModifiedActionCallback)
            resourceWatcher.ResourceModifiedActionCallback(ResourceWatcher::ResourceModified, res->filePath);
          break;
        case WatchedResource::ResourceType_Shader:
          if(resourceWatcher.ResourceModifiedActionCallback)
            resourceWatcher.ResourceModifiedActionCallback(ResourceWatcher::ResourceModified, res->filePath);
          break;
        case WatchedResource::ResourceType_ShaderProgram:
          if(resourceWatcher.ResourceModifiedActionCallback)
            resourceWatcher.ResourceModifiedActionCallback(ResourceWatcher::ResourceModified, res->filePath);
          break;
        case WatchedResource::ResourceType_Texture:
          if(resourceWatcher.ResourceModifiedActionCallback)
            resourceWatcher.ResourceModifiedActionCallback(ResourceWatcher::ResourceModified, res->filePath);
          break;
        case WatchedResource::ResourceType_Mesh:
          if(resourceWatcher.ResourceModifiedActionCallback)
            resourceWatcher.ResourceModifiedActionCallback(ResourceWatcher::ResourceModified, res->filePath);
          break;
        case WatchedResource::ResourceType_Skeleton:
          if(resourceWatcher.ResourceModifiedActionCallback)
            resourceWatcher.ResourceModifiedActionCallback(ResourceWatcher::ResourceModified, res->filePath);
          break;
        default:
          break;
        }

        res->Reload(ResourceWatcher::get());

      }
      else
        std::cout << "ERROR: File modified, but ResourceWatcher aren't aware of it!";

    }
    break;
    //std::cout << "File (" << dir + "\\" + filename << ") Modified! " << std::endl;
    break;
  default:
    break;
  }
}

bool ResourceWatcher::AddResourceDirectory(const boost::filesystem::path& p, bool recursive, bool alsoParseScripts)
{
  using namespace boost::filesystem;

  if(!is_directory(p))
    return false;

  if(directories.count(p))
  {
    if(!recursive) // already added but new call is not recursive
      return false;

    bool r = false;
    recursive_directory_iterator end;
    for(recursive_directory_iterator it(p); it != end; ++it)
    {
      if(is_directory(*it))
      {
        if(directories.count(*it))
          continue;

        if(AddResourceDirectory(*it, false))
          r = true;
      }
    }

    return r;
  }
  else
  {
    FW::WatchID watchid = fileWatcher->addWatch(p.wstring(), fileChangeListener, false);

    // iterate directory and add WatchedResource
    directories[p] = watchid;

    directory_iterator end;
    for(directory_iterator it(p); it != end; ++it)
    {
      if(is_regular_file(*it))
      {
        WatchedResource::ResourceType type = DetermineResourceType(*it);
        if(type != WatchedResource::ResourceType_Unknown)
          watchedFiles[*it].init(*this, watchid, *it, type);
      }
    }

    std::string utf8 = boost::locale::conv::utf_to_utf<char>(p.wstring());
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(utf8, "FileSystem", "General");

    if(alsoParseScripts)
      ParseScriptsInPath(p);

    return true;
  }

  return false;
}

bool ResourceWatcher::WaitTillExclusiveFileAccess(const boost::filesystem::path &filePath, int maxWaitTimeMilliSecs)
{
  using namespace boost::filesystem;
  int count = 0;

  HANDLE hFile = INVALID_HANDLE_VALUE;
  while(hFile == INVALID_HANDLE_VALUE)
  {

    hFile = CreateFile(filePath.wstring().c_str(),                // name of the write
      GENERIC_READ,          // open for writing
      0,                      // do not share
      NULL,                   // default security
      OPEN_EXISTING,             // create new file only
      FILE_ATTRIBUTE_NORMAL,  // normal file
      NULL);                  // no attr. template

    boost::this_thread::sleep(boost::posix_time::milliseconds(10));
    count++;
    if(count * 10 > maxWaitTimeMilliSecs )
    {
      CloseHandle(hFile);
      return false;
    }
  }

  CloseHandle(hFile);

  return true;
}

WatchedResource::ResourceType ResourceWatcher::DetermineResourceType(const boost::filesystem::path& pathToFile)
{
  using namespace boost::filesystem;
  if(is_directory(pathToFile))
    return WatchedResource::ResourceType::ResourceType_Directory;

  std::wstring s = pathToFile.extension().wstring();

  if(s == L".material")
    return WatchedResource::ResourceType_Material;
  else if(s == L".mesh")
    return WatchedResource::ResourceType_Mesh;
  else if(s == L".skeleton")
    return WatchedResource::ResourceType_Skeleton;
  else if(s == L".cg" || s == L".hlsl" || s == L".glsl")
    return WatchedResource::ResourceType_Shader;
  else if(s == L".program") 
    return WatchedResource::ResourceType_ShaderProgram;
  else if(s == L".png" || s == L".tga" || s == L".jpg" || s == L".dds" || s == L".bmp" || s == L".jpeg")
    return WatchedResource::ResourceType_Texture;
  else
    return WatchedResource::ResourceType_Unknown;
}

WatchedResource* ResourceWatcher::GetWatchedResource(const boost::filesystem::path& pathToFile)
{
  using namespace boost::filesystem;

  auto it = watchedFiles.find(pathToFile);

  if(it == watchedFiles.end())
    return 0;

  return &it->second;
}

void ResourceWatcher::ParseFile(const boost::filesystem::path& filePath)
{
  std::string ext = filePath.leaf().extension().string();

  std::string utf8path = "";
  enum ResourceType
  {
    NONE,
    PROGRAM,
    MATERIAL,
    PARTICLE,
    FONT,
    OVERLAY
  };
  ResourceType type = NONE;

  if(ext == ".program")
    type = PROGRAM;
  else if(ext == ".material")
    type = MATERIAL;
  else if(ext == ".fontdef")
    type = FONT;
  else if(ext == ".overlay")
    type = OVERLAY;
  else if(ext == ".particle")
    type = PARTICLE;

  if(type == NONE)
    return;

  utf8path = Ogre::StringConverter::UTF16toUTF8(filePath.wstring());

  switch(type)
  {
  case PROGRAM:
  case MATERIAL:
    try
    {
      Ogre::MaterialManager::getSingleton().parseScript(Ogre::ResourceGroupManager::getSingleton().openResource(utf8path, "General"), "General");
    }catch(...)
    {
      std::cout << "Warning: Material already parsed. " << utf8path << "\n";
    }
    break;
  case PARTICLE:
    try
    {
      Ogre::ParticleSystemManager::getSingleton().parseScript(Ogre::ResourceGroupManager::getSingleton().openResource(utf8path, "General"), "General");
    }catch(...)
    {
      std::cout << "Warning: Particle already parsed. " << utf8path << "\n";
    }
    break;
  case FONT:
    Ogre::FontManager::getSingleton().parseScript(Ogre::ResourceGroupManager::getSingleton().openResource(utf8path, "General"), "General");
    break;
  case OVERLAY:
    Ogre::OverlayManager::getSingleton().parseScript(Ogre::ResourceGroupManager::getSingleton().openResource(utf8path, "General"), "General");
    break;

  }

}

void ResourceWatcher::ParseScriptsInPath(const boost::filesystem::path &p)
{
  enum ResourceType
  {
    PROGRAM,
    MATERIAL,
    PARTICLE,
    FONT,
    OVERLAY
  };

  std::vector<std::string> programFiles;
  std::vector<std::string> materialFiles;
  std::vector<std::pair<ResourceType, std::string> > files;
  using namespace boost::filesystem;

  // parse .program files first
  directory_iterator end;
  for(directory_iterator it(p); it != end; ++it)
  {
    if(is_regular_file(*it))
    {
      const path &filePath = it->path();
      std::string ext = filePath.leaf().extension().string();

      if(ext == ".program")
      {
        std::string s = Ogre::StringConverter::UTF16toUTF8(filePath.wstring());
        programFiles.push_back(s);
      }
      else if(ext == ".material")
      {
        std::string s = Ogre::StringConverter::UTF16toUTF8(filePath.wstring());
        materialFiles.push_back(s);
      }
      else if(ext == ".fontdef")
      {
        std::string s = Ogre::StringConverter::UTF16toUTF8(filePath.wstring());
        files.push_back(std::make_pair(FONT, s));
      }
      else if(ext == ".overlay")
      {
        std::string s = Ogre::StringConverter::UTF16toUTF8(filePath.wstring());
        files.push_back(std::make_pair(PARTICLE, s));
      }
    }
  }

  Ogre::MaterialManager &mm = Ogre::MaterialManager::getSingleton();
  for(auto &i : programFiles)
    mm.parseScript(Ogre::ResourceGroupManager::getSingleton().openResource(i, "General"), "General");

  for(auto &i : materialFiles)
    mm.parseScript(Ogre::ResourceGroupManager::getSingleton().openResource(i, "General"), "General");

  Ogre::FontManager &fm = Ogre::FontManager::getSingleton();
  for(auto &i : materialFiles)
    Ogre::FontManager::getSingleton().parseScript(Ogre::ResourceGroupManager::getSingleton().openResource(i, "General"), "General");

  Ogre::ParticleSystemManager &pm = Ogre::ParticleSystemManager::getSingleton();
  Ogre::OverlayManager &om = Ogre::OverlayManager::getSingleton();
  for(auto &i : files)
  {
    switch(i.first)
    {
    case PARTICLE:
      pm.parseScript(Ogre::ResourceGroupManager::getSingleton().openResource(i.second, "General"), "General");
      break;
    case OVERLAY:
      om.parseScript(Ogre::ResourceGroupManager::getSingleton().openResource(i.second, "General"), "General");
      break;
    default:
      break;
    }
  }

}

void ResourceWatcher::init()
{
  fileWatcher = new FW::FileWatcher();
  fileChangeListener = new FileChangeListener();

  // calling this to be on the safe side.
  // Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

void ResourceWatcher::destroy()
{
  delete fileChangeListener;
  delete fileWatcher;
}

void ResourceWatcher::FindSubEntitiesUsingMaterialFile(const boost::filesystem::path &filePath, std::vector<std::pair<Ogre::SubEntity*, Ogre::String> >& subEntitiesToBeReloaded)
{
  for(auto scene : scenes)
  {
    auto it = scene->getMovableObjectIterator("Entity");

    while(it.hasMoreElements())
    {
      Ogre::MovableObject *obj = it.getNext();

      if(dynamic_cast<Ogre::Entity*>(obj))
      {
        Ogre::Entity *e = dynamic_cast<Ogre::Entity*>(obj);
        int size = e->getNumSubEntities();
        for(int i = 0; i < size; ++i)
        {
          Ogre::SubEntity *sub = e->getSubEntity(i);

          Ogre::String matName = sub->getMaterialName();
          auto it = resourceInfos->resourceFiles.find(matName);
          if(it != resourceInfos->resourceFiles.end())
          {
            if(it->second == filePath)
            {
              sub->setMaterialName("BaseWhite");
              subEntitiesToBeReloaded.push_back(std::make_pair(sub, matName));
            }
          }
        }
      }
    }
  }
}

void ResourceWatcher::FindRenderablesUsingMaterial(const std::string &materialName, std::set<Ogre::Renderable*> &renderables)
{
  for(auto scene : scenes)
  {
    //TODO: find Renderables other than entity
    auto it = scene->getMovableObjectIterator("Entity");

    while(it.hasMoreElements())
    {
      Ogre::MovableObject *obj = it.getNext();

      if(dynamic_cast<Ogre::Entity*>(obj))
      {
        Ogre::Entity *e = dynamic_cast<Ogre::Entity*>(obj);
        int size = e->getNumSubEntities();
        for(int i = 0; i < size; ++i)
        {
          Ogre::SubEntity *sub = e->getSubEntity(i);

          Ogre::String matName = sub->getMaterialName();

          if(matName == materialName)
          {
            sub->setMaterialName("BaseWhite");
            renderables.insert(sub);
          }
        }
      }
    }
  }
}

void WatchedResource::ReloadShader(ResourceWatcher& watcher)
{
  std::set<std::string> shaders;

  // holds shaders and materials that use them
  std::unordered_map<Ogre::GpuProgram*, std::set<std::pair<std::string, std::pair<int, int>> > > programsToBeReloaded;

  std::string resourceFileName = filePath.leaf().string();

  for(auto scene : watcher.scenes)
  {
    auto it = scene->getMovableObjectIterator("Entity");

    while(it.hasMoreElements())
    {
      Ogre::MovableObject *obj = it.getNext();

      if(dynamic_cast<Ogre::Entity*>(obj))
      {
        Ogre::Entity *e = dynamic_cast<Ogre::Entity*>(obj);
        unsigned int size = e->getNumSubEntities();
        for(unsigned int i = 0; i < size; ++i)
        {
          Ogre::SubEntity *sub = e->getSubEntity(i);

          Ogre::Material *mat = sub->getMaterial().get();
          unsigned int numTech = mat->getNumTechniques();

          for(unsigned int i = 0; i < numTech; ++i)
          {
            unsigned int numPass = mat->getTechnique(i)->getNumPasses();
            for(unsigned int y = 0; y < numPass; ++y)
            {
              Ogre::Pass *p = mat->getTechnique(i)->getPass(y);

              int techniqueIndex = i;

              int passIndex = p->getIndex();

              if(p->hasVertexProgram())
              {
                Ogre::GpuProgram *vertex = p->getVertexProgram().get();

                const Ogre::String &name = vertex->getSourceFile();

                if(resourceFileName == name)
                {
                  programsToBeReloaded[vertex].insert(std::make_pair(mat->getName(), std::make_pair(techniqueIndex, passIndex)));
                }
              }

              if(p->hasFragmentProgram())
              {
                Ogre::GpuProgram *pixel = p->getFragmentProgram().get();

                const Ogre::String &name = pixel->getSourceFile();

                if(resourceFileName == name)
                {
                  programsToBeReloaded[pixel].insert(std::make_pair(mat->getName(), std::make_pair(techniqueIndex, passIndex)));
                }
              }
            }

          }
        }
      }
    }
  }

  // RELOAD SHADERS
  for(auto it = programsToBeReloaded.begin(); it != programsToBeReloaded.end(); ++it)
  {
    ResourceWatcher::get().ReloadGpuProgram(it->first, it->second);
  }
}

void WatchedResource::ReloadProgram()
{
  ResourceWatcher &watcher = ResourceWatcher::get();

  boost::unordered_map<Ogre::GpuProgram*, std::set<std::pair<std::string, std::pair<int, int>> > > programs;

  std::set<Ogre::String> materialsUsingProgram;
  std::set<Ogre::Renderable*> renderables;

  std::string filename = filePath.leaf().string();

  for(auto scene : watcher.scenes)
  {
    auto it = scene->getMovableObjectIterator("Entity");

    while(it.hasMoreElements())
    {
      Ogre::MovableObject *obj = it.getNext();

      if(dynamic_cast<Ogre::Entity*>(obj))
      {
        Ogre::Entity *e = dynamic_cast<Ogre::Entity*>(obj);
        unsigned int size = e->getNumSubEntities();
        for(unsigned int i = 0; i < size; ++i)
        {
          Ogre::SubEntity *sub = e->getSubEntity(i);

          Ogre::Material *mat = sub->getMaterial().get();
          unsigned int numTech = mat->getNumTechniques();

          for(unsigned int i = 0; i < numTech; ++i)
          {
            unsigned int numPass = mat->getTechnique(i)->getNumPasses();
            for(unsigned int y = 0; y < numPass; ++y)
            {
              Ogre::Pass *p = mat->getTechnique(i)->getPass(y);

              if(p->hasVertexProgram())
              {
                if(p->getVertexProgram()->getOrigin() == filename)
                {
                  materialsUsingProgram.insert(mat->getName());
                  renderables.insert(sub);
                }
              }

              if(p->hasFragmentProgram())
              {
                if(p->getFragmentProgram()->getOrigin() == filename)
                {
                  materialsUsingProgram.insert(mat->getName());
                  renderables.insert(sub);
                }
              }
            }

          }
        }
      }
    }
  }

  for(auto &matname : materialsUsingProgram)
    ResourceWatcher::get().CompleteReloadOfMaterial(matname, renderables);

}

void WatchedResource::Reload(ResourceWatcher &watcher)
{
  switch(resourceType)
  {
  case WatchedResource::ResourceType_Unknown:
    break;
  case WatchedResource::ResourceType_Material:
    ReloadMaterial(watcher);
    break;
  case WatchedResource::ResourceType_Shader:
    ReloadShader(watcher);
    break;
  case WatchedResource::ResourceType_ShaderProgram:
    //TODO: reload .program files
    ReloadProgram();
    break;
  case WatchedResource::ResourceType_Texture:
    ReloadTexture();
    break;
  case WatchedResource::ResourceType_Mesh:
    ReloadMesh(watcher);
    break;
  case WatchedResource::ResourceType_Skeleton:
    break;
  default:
    break;
  }
}

void WatchedResource::ReloadMaterial(ResourceWatcher &watcher)
{

  std::vector<std::pair<Ogre::SubEntity*, Ogre::String>> subEntitiesToBeReloaded;

  ResourceWatcher::get().FindSubEntitiesUsingMaterialFile(filePath, subEntitiesToBeReloaded);

  // Remove each material in defined file
  Ogre::MaterialManager &mm = Ogre::MaterialManager::getSingleton();
  for(unsigned i = 0; i < components.size(); ++i)
  {
    if(components[i].first == ComponentType_Material)
      mm.remove(components[i].second);
  }

  // parse script again
  try
  {
    std::string s = Ogre::StringConverter::UTF16toUTF8(filePath.wstring());
    Ogre::MaterialManager::getSingleton().parseScript(Ogre::ResourceGroupManager::getSingleton().openResource(s, "General"), "General");
  }
  catch(...)
  {
    std::cout << "ERROR: Material not loaded, can't reload.\n";
  }

  // set original materials
  for(unsigned j = 0; j < subEntitiesToBeReloaded.size(); ++j)
  {
    try
    {
      static_cast<Ogre::SubEntity*>(subEntitiesToBeReloaded[j].first)->setMaterialName(subEntitiesToBeReloaded[j].second);
    }
    catch(...)
    {
      // happens if ogre can't decode image
      int a = 2;
      continue;
    }
    if(ResourceWatcher::get().ResourceReloadedCallback)
      ResourceWatcher::get().ResourceReloadedCallback(ResourceWatcher::get().ReloadType_Material, &subEntitiesToBeReloaded[j].second);
  }

}

void WatchedResource::init(ResourceWatcher &watcher, FW::WatchID watchId, const boost::filesystem::path &filePath, ResourceType type)
{
  using namespace boost::filesystem;

  this->watchId = watchId, this->filePath = filePath;

  resourceType = type;

  switch(resourceType)
  {
  case ResourceType_Material:
  case ResourceType_ShaderProgram:
    FillComponents();
    break;
  default:
    {
      // TODO, too many string operations
      std::string s = Ogre::StringConverter::UTF16toUTF8(filePath.leaf().wstring());
      // Ogre::StringUtil::toLowerCase(s);
      ResourceInfos::get().resourceFiles[s] = filePath;
    }
    break;
  }

}

void WatchedResource::FillComponents()
{
  using namespace boost::filesystem;

  ResourceWatcher &watcher = ResourceWatcher::get();

  switch(resourceType)
  {
  case ResourceType_Material:
    {
      ifstream f(filePath);
      std::string s;
      while(getline(f, s, '\n'))
      {
        {
          int loc = s.find("material");

          if(loc != -1)
          {
            int baseLoc = s.find(':');

            std::string base;
            std::string matName;

            if(baseLoc != -1)
              base = s.substr(baseLoc + 1, s.length());

            if(baseLoc == -1)
              matName = s.substr(loc + 8, s.length());
            else
              matName = s.substr(loc + 8, baseLoc - (loc + 8));

            if(baseLoc != -1)
            {
              Ogre::StringUtil::trim(base);
              components.push_back(std::make_pair(ComponentType_Material, base));
            }

            Ogre::StringUtil::trim(matName);
            components.push_back(std::make_pair(ComponentType_Material, matName));
            ResourceInfos::get().resourceFiles[matName] = filePath;
          }
        }
      }
    }
    break;
  case ResourceType_ShaderProgram:
    {
      ifstream f(filePath);
      std::string s;
      while(getline(f, s, '\n'))
      {

        int loc = s.find("vertex_program");

        if(loc != -1)
        {
          std::string name = s.substr(loc + 15, s.length());
          loc = name.find(' ');
          name = name.substr(0, loc);

          Ogre::StringUtil::trim(name);
          components.push_back(std::make_pair(ComponentType_VertexProgram, name));
          ResourceInfos::get().resourceFiles[name] = filePath;
        }

        loc = s.find("fragment_program");

        if(loc != -1)
        {
          std::string name = s.substr(loc + 17, s.length());
          loc = name.find(' ');
          name = name.substr(0, loc);

          Ogre::StringUtil::trim(name);
          components.push_back(std::make_pair(ComponentType_FragmentProgram, name));
          ResourceInfos::get().resourceFiles[name] = filePath;
        }

      }
    }
    break;
  default:
    break;
  }

}

void WatchedResource::ReloadTexture()
{
  bool r = ResourceWatcher::WaitTillExclusiveFileAccess(filePath, 3000);
  if(!r)
  {
    std::cout << "Error: Could not read file: " << filePath.string() << "\n";
    return;
  }

  std::string textureName = filePath.leaf().string();
  Ogre::TextureManager &tm = Ogre::TextureManager::getSingleton();
  Ogre::TexturePtr p = tm.getByName(textureName);

  if(p.isNull())
    return;

  try
  {
    p->reload();
  }
  catch(...)
  {
    //TODO: show log that it could open file
    int a = 2;
  }
  if(ResourceWatcher::get().ResourceReloadedCallback)
    ResourceWatcher::get().ResourceReloadedCallback(ResourceWatcher::ReloadType_Texture, p.get());
}

void WatchedResource::ReloadMesh(ResourceWatcher &watcher)
{
  std::string meshName = filePath.leaf().string();
  // Ogre::StringUtil::toLowerCase(meshName);

  std::vector<std::pair<Ogre::Node*, EntityInfo>> entitiesToBeReloaded;
  for(auto scene : watcher.scenes)
  {
    auto it = scene->getMovableObjectIterator("Entity");

    while(it.hasMoreElements())
    {
      Ogre::MovableObject *obj = it.getNext();

      if(dynamic_cast<Ogre::Entity*>(obj))
      {
        Ogre::Entity *e = dynamic_cast<Ogre::Entity*>(obj);

        if(e->getMesh()->getName() ==  meshName)
        {
          entitiesToBeReloaded.push_back(std::make_pair(e->getParentNode(), EntityInfo(e)));
          e->detachFromParent();
          scene->destroyEntity(e);
        }
      }
    }
  }

  Ogre::MeshManager::getSingleton().remove(meshName);

  //wait for mesh file to be accessible
  {
    bool r = ResourceWatcher::WaitTillExclusiveFileAccess(filePath, 3000);
    if(!r)
      std::cout << "Error: can not access mesh file: " << filePath.string() << "\n";
  }

  Ogre::Entity *newEntity = 0;
  for(auto it = entitiesToBeReloaded.begin(); it != entitiesToBeReloaded.end(); ++it)
  {
    {
      Ogre::SceneNode *n = dynamic_cast<Ogre::SceneNode*>(it->first);
      if(n)
      {
        newEntity = it->second.CreateEntity(n->getCreator());
        n->attachObject(newEntity);

        if(ResourceWatcher::get().EntityReloaded && newEntity)
          ResourceWatcher::get().EntityReloaded(it->second.oldEntity, newEntity);

        continue;
      }
    }

    {
      Ogre::TagPoint *n = dynamic_cast<Ogre::TagPoint*>(it->first);
      if(n)
      {
        Ogre::SceneNode *node = static_cast<Ogre::SceneNode*>(n->getParent());
        newEntity = it->second.CreateEntity(node->getCreator());
        node->attachObject(newEntity);

        if(ResourceWatcher::get().EntityReloaded && newEntity)
          ResourceWatcher::get().EntityReloaded(it->second.oldEntity, newEntity);

        continue;
      }
    }
  }
  if(ResourceWatcher::get().ResourceReloadedCallback && newEntity)
    ResourceWatcher::get().ResourceReloadedCallback(ResourceWatcher::ReloadType_Mesh, newEntity);

}

Ogre::Entity * EntityInfo::CreateEntity(Ogre::SceneManager *mgr)
{
  Ogre::Entity *e = mgr->createEntity(meshName);

  /*
  if(materials.empty() || e->getNumSubEntities() != materials.size())
  {
  e->setMaterialName("BaseWhite");
  return e;
  }

  unsigned size = e->getNumSubEntities();
  for(unsigned j = 0; j < size; ++j)
  e->getSubEntity(j)->setMaterialName(materials[j]);
  */
  return e;
}

EntityInfo::EntityInfo(Ogre::Entity *entity)
{
  /*
  unsigned size = entity->getNumSubEntities();
  for(unsigned j = 0; j < size; ++j)
  {
  Ogre::SubEntity *e = entity->getSubEntity(j);
  Ogre::String matName = e->getMaterialName();
  materials.push_back(matName);
  }
  */
  oldEntity = entity;
  meshName = entity->getMesh()->getName();
}

void ResourceWatcher::AddScene(Ogre::SceneManager* scene)
{
  scenes.push_back(scene);
}

void ResourceWatcher::ReloadMaterial(const Ogre::String &matName)
{
  std::vector<std::pair<Ogre::SubEntity*, Ogre::String>> subEntitiesToBeReloaded;

  using namespace boost::filesystem;

  auto it = resourceInfos->resourceFiles.find(matName);

  if(it == resourceInfos->resourceFiles.end())
    return;

  FindSubEntitiesUsingMaterialFile(it->second, subEntitiesToBeReloaded);

  for(auto i : subEntitiesToBeReloaded)
    i.first->setMaterialName("BaseWhite");

  // parse script again
  try
  {
    std::string s = Ogre::StringConverter::UTF16toUTF8(it->second.wstring());
    Ogre::MaterialManager::getSingleton().parseScript(Ogre::ResourceGroupManager::getSingleton().openResource(s, "General"), "General");
  }
  catch(...)
  {
    std::cout << "ERROR: Material not loaded, can't reload.\n";
  }

  // set original materials
  for(unsigned j = 0; j < subEntitiesToBeReloaded.size(); ++j)
  {
    static_cast<Ogre::SubEntity*>(subEntitiesToBeReloaded[j].first)->setMaterialName(subEntitiesToBeReloaded[j].second);
  }
}

void ResourceWatcher::SetSubEntitiesToBaseWhite(const std::string& materialName, std::vector<std::pair<Ogre::SubEntity*, Ogre::String> >& subEntitiesToBeReloaded)
{
  using namespace boost::filesystem;

  auto it = resourceInfos->resourceFiles.find(materialName);

  if(it == resourceInfos->resourceFiles.end())
    return;

  FindSubEntitiesUsingMaterialFile(it->second, subEntitiesToBeReloaded);

  for(auto i : subEntitiesToBeReloaded)
    i.first->setMaterialName("BaseWhite");
}

void ResourceWatcher::ReloadGpuProgram(Ogre::GpuProgram* shader, std::set<std::pair<std::string, std::pair<int, int>> > &passes)
{

  Ogre::HighLevelGpuProgramManager *gpm = Ogre::HighLevelGpuProgramManager::getSingletonPtr();
  Ogre::ResourceGroupManager* rgm = Ogre::ResourceGroupManager::getSingletonPtr();
  Ogre::MaterialManager* mm = Ogre::MaterialManager::getSingletonPtr();

  for(auto it = passes.begin(); it != passes.end(); ++it)
  {

    int techniqueIndex = it->second.first;
    int passIndex = it->second.second;
    auto shader_type = shader->getType();

    Ogre::Material *mat = static_cast<Ogre::Material*>(Ogre::MaterialManager::getSingleton().getByName(it->first).get());
    Ogre::Pass *pass = mat->getTechnique(it->second.first)->getPass(it->second.second);

    if(!shader->isLoaded())
    {
      Ogre::String matName = mat->getName();
      CompleteReloadOfMaterial(matName);
      // CompleteReloadOfShader(shader->getName());
    }
    else
    {
      if(ResourceWatcher::get().CanShaderCompile(shader))
      {
        Ogre::GpuProgramParametersSharedPtr old_params;

        switch(shader->getType())
        {
        case Ogre::GPT_VERTEX_PROGRAM:
          old_params = pass->getVertexProgramParameters();
          break;
        case Ogre::GPT_FRAGMENT_PROGRAM:
          old_params = pass->getFragmentProgramParameters();
          break;
        default:
          break;
        }

        shader->reload();

        switch(shader->getType())
        {
        case Ogre::GPT_VERTEX_PROGRAM:
          pass->getVertexProgramParameters().get()->copyMatchingNamedConstantsFrom(*old_params.get());
          break;
        case Ogre::GPT_FRAGMENT_PROGRAM:
          pass->getFragmentProgramParameters().get()->copyMatchingNamedConstantsFrom(*old_params.get());
          break;
        default:
          break;
        }

        if(ResourceReloadedCallback)
          ResourceReloadedCallback(ReloadType_Shader, shader);
      }

    }

  }

}

void ResourceWatcher::CompleteReloadOfMaterial(const std::string& materialName, std::set<Ogre::Renderable*> &renderables)
{
  Ogre::MaterialManager &mm = Ogre::MaterialManager::getSingleton();
  Ogre::HighLevelGpuProgramManager &gpm = Ogre::HighLevelGpuProgramManager::getSingleton();
  Ogre::ResourceGroupManager &rgm = Ogre::ResourceGroupManager::getSingleton();

  Ogre::Material *mat = static_cast<Ogre::Material*>(mm.getByName(materialName).get());

  if(!mat)
    return;

  Ogre::String filename = mat->getOrigin();

  std::vector<Ogre::GpuProgram*> gpuPrograms;

  std::set<Ogre::String> programFiles;

  struct ShaderInfo
  {
    Ogre::String shaderName;
    int techniqueIndex;
    int passIndex;
    Ogre::GpuProgramType type;
    Ogre::GpuProgramParameters old_params;
  };

  std::set<Ogre::String> shaderFiles;
  std::vector<ShaderInfo> shaderInfos;

  for(int i = 0; i < mat->getNumTechniques(); ++i)
  {
    for(int y = 0; y < mat->getTechnique(i)->getNumPasses(); ++y)
    {
      Ogre::Pass *pass = mat->getTechnique(i)->getPass(y);
      if(pass->hasVertexProgram())
      {
        Ogre::GpuProgram *program = pass->getVertexProgram().get();
        gpuPrograms.push_back(program);
        programFiles.insert(program->getOrigin());
        shaderFiles.insert(program->getSourceFile());
        shaderInfos.push_back(ShaderInfo());
        ShaderInfo &info = shaderInfos.back();
        info.shaderName = program->getName();
        info.techniqueIndex = i;
        info.passIndex = y;
        info.type = program->getType();

        info.old_params = *(pass->getVertexProgramParameters().get());
      }
      if(pass->hasFragmentProgram())
      {
        Ogre::GpuProgram *program = pass->getFragmentProgram().get();
        gpuPrograms.push_back(program);
        programFiles.insert(program->getOrigin());
        shaderFiles.insert(program->getSourceFile());
        shaderInfos.push_back(ShaderInfo());
        ShaderInfo &info = shaderInfos.back();
        info.shaderName = program->getName();
        info.techniqueIndex = i;
        info.passIndex = y;
        info.type = program->getType();

        info.old_params = *(pass->getFragmentProgramParameters().get());
      }
    }
  }

  for(auto entity : renderables)
  {
    {
      Ogre::SubEntity *e = dynamic_cast<Ogre::SubEntity*>(entity);
      if(e)
        e->setMaterialName("BaseWhite");
    }
  }

  mm.remove(materialName);

  for(auto &info : shaderInfos)
    gpm.remove(info.shaderName);

  for(auto &shaderName : shaderFiles)
    mm.parseScript(rgm.openResource(shaderName, "General"), "General");

  for(auto &programFile : programFiles)
  {
    if(filename != programFile)
      mm.parseScript(rgm.openResource(programFile, "General"), "General");
  }

  // re-parse script
  mm.parseScript(rgm.openResource(filename, "General"), "General");

  for(auto entity : renderables)
  {
    {
      Ogre::SubEntity *e = dynamic_cast<Ogre::SubEntity*>(entity);
      if(e)
        e->setMaterialName(materialName);
    }
  }

  // re-get material. because previous one should be removed
  mat = static_cast<Ogre::Material*>(mm.getByName(materialName, "General").get());

  for(auto &info : shaderInfos)
  {
    Ogre::GpuProgram *shader;
    switch(info.type)
    {
    case Ogre::GPT_VERTEX_PROGRAM:
      shader = mat->getTechnique(info.techniqueIndex)->getPass(info.passIndex)->getVertexProgram().get();
      shader->resetCompileError();
      shader->reload();
      mat->getTechnique(info.techniqueIndex)->getPass(info.passIndex)->getVertexProgramParameters().get()->copyMatchingNamedConstantsFrom(info.old_params);
      if(ResourceWatcher::get().ResourceReloadedCallback)
        ResourceWatcher::get().ResourceReloadedCallback(ResourceWatcher::ReloadType_Shader, shader);
      break;
    case Ogre::GPT_FRAGMENT_PROGRAM:
      shader = mat->getTechnique(info.techniqueIndex)->getPass(info.passIndex)->getFragmentProgram().get();
      shader->resetCompileError();
      shader->reload();
      mat->getTechnique(info.techniqueIndex)->getPass(info.passIndex)->getFragmentProgramParameters().get()->copyMatchingNamedConstantsFrom(info.old_params);
      if(ResourceWatcher::get().ResourceReloadedCallback)
        ResourceWatcher::get().ResourceReloadedCallback(ResourceWatcher::ReloadType_Shader, shader);
      break;
    default:
      break;
    }
  }
}

void ResourceWatcher::CompleteReloadOfMaterial(const std::string& materialName)
{
  std::set<Ogre::Renderable*> renderables;
  ResourceWatcher::get().FindRenderablesUsingMaterial(materialName, renderables);

  CompleteReloadOfMaterial(materialName, renderables);
}

bool ResourceWatcher::CanShaderCompile(Ogre::GpuProgram* program)
{
  Ogre::HighLevelGpuProgramManager *gpm = Ogre::HighLevelGpuProgramManager::getSingletonPtr();

  Ogre::HighLevelGpuProgramPtr temp_vert = gpm->createProgram("__compile_check", "General", program->getLanguage(), program->getType());

  program->copyParametersTo(temp_vert.get());
  temp_vert->setSourceFile(program->getSourceFile());

  temp_vert->load();

  bool hasError = temp_vert->hasCompileError();

  if(hasError)
  {
    if(ShaderCompileFailed)
    {
      //TODO, somehow also tell about the error type
      ShaderCompileFailed(program->getName());
    }
  }

  temp_vert.setNull();
  gpm->remove("__compile_check");

  return !hasError;
}

void ResourceWatcher::CompleteReloadOfShader(std::string shaderName)
{
  Ogre::MaterialManager &mm = Ogre::MaterialManager::getSingleton();
  Ogre::HighLevelGpuProgramManager &gpm = Ogre::HighLevelGpuProgramManager::getSingleton();
  Ogre::ResourceGroupManager &rgm = Ogre::ResourceGroupManager::getSingleton();

  struct ShaderInfo
  {
    Ogre::Material *material;
    int techniqueIndex;
    int passIndex;
    Ogre::GpuProgramType type;
    Ogre::GpuProgramParameters old_params;
  };

  std::set<Ogre::String> shaderFiles;
  std::vector<ShaderInfo> shaderInfos;

  auto matIt = mm.getResourceIterator();

  while(matIt.hasMoreElements())
  {
    Ogre::Material *mat = static_cast<Ogre::Material*>(matIt.getNext().get());

    for(int i = 0; i < mat->getNumTechniques(); ++i)
    {
      for(int y = 0; y < mat->getTechnique(i)->getNumPasses(); ++y)
      {
        Ogre::Pass *pass = mat->getTechnique(i)->getPass(y);
        if(pass->hasVertexProgram())
        {

          if(pass->getVertexProgramName() == shaderName)
          {
            Ogre::GpuProgram *program = pass->getVertexProgram().get();
            shaderInfos.push_back(ShaderInfo());
            ShaderInfo &info = shaderInfos.back();
            info.material = mat;
            info.techniqueIndex = i;
            info.passIndex = y;
            info.type = program->getType();
            info.old_params = *(pass->getVertexProgramParameters().get());

            pass->setVertexProgram("");
          }
        }
        if(pass->hasFragmentProgram())
        {
          if(pass->getFragmentProgramName() == shaderName)
          {

            Ogre::GpuProgram *program = pass->getFragmentProgram().get();
            shaderInfos.push_back(ShaderInfo());
            ShaderInfo &info = shaderInfos.back();
            info.material = mat;
            info.techniqueIndex = i;
            info.passIndex = y;
            info.type = program->getType();
            info.old_params = *(pass->getFragmentProgramParameters().get());

            pass->setFragmentProgram("");

          }
        }
      }
    }

  }

  Ogre::HighLevelGpuProgramPtr shader = static_cast<Ogre::HighLevelGpuProgramPtr>(gpm.getByName(shaderName));

  std::map<std::string, std::string> params;
  for(auto i : shader->getParameters())
    params[i.name] = shader->getParameter(i.name);

  Ogre::GpuProgramType type = shader->getType();
  Ogre::String lang = shader->getLanguage();
  std::string filePath = shader->getSourceFile();
  std::string origin = shader->getOrigin();

  shader.setNull();
  gpm.remove(shaderName);

  shader = gpm.createProgram(shaderName, "General", lang, type);
  shader->setSourceFile(filePath);

  mm.parseScript(rgm.openResource(origin, "General"), "General");

  for(auto &i : params)
    shader->setParameter(i.first, i.second);

  shader->load();

  for(auto &info : shaderInfos)
  {
    switch(info.type)
    {
    case Ogre::GPT_VERTEX_PROGRAM:
      info.material->getTechnique(info.techniqueIndex)->getPass(info.passIndex)->setVertexProgram(shaderName);
      shader->resetCompileError();
      shader->reload();
      info.material->getTechnique(info.techniqueIndex)->getPass(info.passIndex)->getVertexProgramParameters().get()->copyMatchingNamedConstantsFrom(info.old_params);
      if(ResourceWatcher::get().ResourceReloadedCallback)
        ResourceWatcher::get().ResourceReloadedCallback(ResourceWatcher::ReloadType_Shader, shader.get());
      break;
    case Ogre::GPT_FRAGMENT_PROGRAM:
      info.material->getTechnique(info.techniqueIndex)->getPass(info.passIndex)->setFragmentProgram(shaderName);
      shader->resetCompileError();
      shader->reload();
      info.material->getTechnique(info.techniqueIndex)->getPass(info.passIndex)->getFragmentProgramParameters().get()->copyMatchingNamedConstantsFrom(info.old_params);
      if(ResourceWatcher::get().ResourceReloadedCallback)
        ResourceWatcher::get().ResourceReloadedCallback(ResourceWatcher::ReloadType_Shader, shader.get());
      break;
    default:
      break;
    }
  }

}

void ResourceWatcher::OpenResource(const std::string& resourceName)
{
  auto it = resourceInfos->resourceFiles.find(resourceName);

  if(it == resourceInfos->resourceFiles.end())
    return;

  ShellExecute(0, 0, it->second.wstring().c_str(), 0, 0, SW_SHOW);
}

void ResourceWatcher::OpenShaderFile(const std::string& shaderFile)
{
  Ogre::GpuProgramPtr shader = Ogre::GpuProgramManager::getSingleton().getByName(shaderFile);

  if(shader.isNull())
    return;

  for(auto &it : resourceInfos->resourceFiles)
  {
    if(it.second.leaf() == shader->getSourceFile())
    {
      ShellExecute(0, 0, it.second.wstring().c_str(), 0, 0, SW_SHOW);
    }
  }
}

void ResourceWatcher::UpdateResourceDirectory(const boost::filesystem::path& p)
{
  std::string utf8 = boost::locale::conv::utf_to_utf<char>(p.wstring());
  Ogre::ResourceGroupManager::getSingleton().addResourceLocation(utf8, "FileSystem", "General");
}

void ResourceWatcher::RemoveResourceDirectory(const boost::filesystem::path& p, bool removeChildren)
{
  if(!removeChildren)
  {
    auto it = directories.find(p);

    if(it != directories.end())
    {
      std::string utf8 = boost::locale::conv::utf_to_utf<char>(p.wstring());
      Ogre::ResourceGroupManager::getSingleton().removeResourceLocation(utf8, "General");
      fileWatcher->removeWatch(it->second);
      directories.erase(it);
      // TODO: remove from watched, existing WatchedResource doesn't cause any mischief but uses memory
    }
  }
  else
  {
    auto it = directories.find(p);

    if(it != directories.end())
    {
      using namespace boost::filesystem;

      recursive_directory_iterator end;
      for(recursive_directory_iterator child(it->first); child != end; ++child)
      {
        if(is_directory(*child))
          RemoveResourceDirectory(*child);
      }
      // remove directory itself, bacuse resursive iteretor does not iterate main path given
      RemoveResourceDirectory(p);
    }

  }
}

const boost::filesystem::path* ResourceWatcher::GetResourcePath(const Ogre::String& name)
{
  auto it = resourceInfos->resourceFiles.find(name);

  if(it == resourceInfos->resourceFiles.end())
    return nullptr;

  return &it->second;
}

void ResourceWatcher::AddResourceDirectoryToQueue(const boost::filesystem::path& p, bool isRecursive)
{
  using namespace boost::filesystem;

  if(!is_directory(p))
  {
    std::wcout << L"Can not add, " << p.wstring() << L" is not a directory\n";
    return;
  }

  boost::filesystem::path npath = p;
  npath.make_preferred();

  queuedResourceDirectories.push_back(std::move(std::make_pair( npath, isRecursive)));
}

void ResourceWatcher::ParseQueuedResourceDirectories()
{

  using namespace boost::filesystem;

  std::vector<path> programFiles;
  std::vector<path> materialFiles;
  std::vector<path> otherResources;

  // make contents unique
  {
    std::vector<std::pair<path,bool> > uniqueItems;

    for(auto it = queuedResourceDirectories.begin(); it != queuedResourceDirectories.end(); ++it )
    {
      bool isUnique = true;
      for(auto &p : uniqueItems)
      {
        if(p.first == it->first)
        {
          isUnique = false;
          break;
        }
      }

      if(isUnique)
        uniqueItems.push_back(*it);
    }

    queuedResourceDirectories = uniqueItems;
  }

  for(auto &dir : queuedResourceDirectories)
    AddResourceDirectory(dir.first, dir.second, false);

  for(auto &dir : queuedResourceDirectories)
  {

    if(dir.second) // recursive
    {
      recursive_directory_iterator end;
      for(recursive_directory_iterator it(dir.first); it != end; ++it)
      {
        std::string ext = it->path().extension().string();
        if(ext == ".program")
          programFiles.push_back(it->path());
        else if(ext == ".material")
          materialFiles.push_back(it->path());
        else if(ext == ".particle" || ext == ".fontdef" || ext == ".overlay")
          otherResources.push_back(it->path());
      }
    }
    else 
    {
      directory_iterator end;
      for(directory_iterator it(dir.first); it != end; ++it)
      {
        std::string ext = it->path().extension().string();
        if(ext == ".program")
          programFiles.push_back(it->path());
        else if(ext == ".material")
          materialFiles.push_back(it->path());
        else if(ext == ".particle" || ext == ".fontdef" || ext == ".overlay")
          otherResources.push_back(it->path());
      }
    }
  }

  // TODO: determine dependencies between material and program files. Call them in correct order

  for(auto &program : programFiles )
    ParseFile(program);

  for(auto &material : materialFiles )
    ParseFile(material);

  for(auto &other : otherResources)
    ParseFile(other);

  queuedResourceDirectories.clear();
}

void ResourceWatcher::RemoveScene(Ogre::SceneManager* scene)
{
  for(int i = 0, size = scenes.size(); i < size; ++i)
  {
    if(scenes[i] == scene)
    {
      scenes.erase(scenes.begin() + i);
      return;
    }
  }
}
