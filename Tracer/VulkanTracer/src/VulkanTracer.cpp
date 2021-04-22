#include "VulkanTracer.h"
#include <cmath>



VulkanTracer::VulkanTracer() {
	bufferSizes.resize(5);
	buffers.resize(5);
	bufferMemories.resize(5);
	//beamline.resize(0);

}

VulkanTracer::~VulkanTracer() {

}


//	This function creates a debug messenger
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

//	This function destroys the debug messenger
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}

/*	this function is used to start the tracer
*/
void VulkanTracer::run()
{
	// At first, the amount of rays is set. This is a placeholder, as the rays won't be generated in this tracer in the future
	setRayAmount();
	std::cout << "Amount of Rays: " << rayAmount << std::endl;
	// generate the rays used by the tracer
	//generateRays();
	//the sizes of the input and output buffers are set. The buffers need to be the size rayamount * size of a Ray * the size of a double
	//(a ray consists of 7 values in double precision, x,y,z for the position and x*, y*, z* for the direction and a weight. 8 values instead of 7 are used, because the shader size of the buffer needs to be multiples of 32 bit)

	//staging buffers need to be at the end of the buffer vector!
	bufferSizes[0] = (uint64_t)rayAmount * VULKANTRACER_RAY_DOUBLE_AMOUNT * sizeof(double);
	bufferSizes[1] = (uint64_t)rayAmount * VULKANTRACER_RAY_DOUBLE_AMOUNT * sizeof(double);
	bufferSizes[2] = beamline.size() * sizeof(double);
	bufferSizes[3] = std::min((uint64_t) GPU_MAX_STAGING_SIZE, (uint64_t)rayAmount * VULKANTRACER_RAY_DOUBLE_AMOUNT * sizeof(double)); //maximum of 128MB
	bufferSizes[4] = std::min((uint64_t) GPU_MAX_STAGING_SIZE, (uint64_t)rayAmount * VULKANTRACER_RAY_DOUBLE_AMOUNT * sizeof(double)); //maximum of 128MB
	for (int i = 0; i<bufferSizes.size(); i++){
		std::cout << "bufferSizes["<<i<<"]: " << bufferSizes[i] << std::endl;
	}
	//vulkan is initialized
	initVulkan();
	mainLoop();
}

//function for initializing vulkan
void VulkanTracer::initVulkan()
{
	//a vulkan instance is created
	createInstance();

	setupDebugMessenger();

	//physical device for computation is chosen
	pickPhysicalDevice();

	//a logical device is created
	//it is needed to communicate with the physical device
	createLogicalDevice();

	//create command pool which will be used to submit the staging buffer
	createCommandPool();

	//creates buffers to transfer data to and from the shader
	createBuffers();
	fillRayBuffer();
	fillQuadricBuffer();
	//creates the descriptors used to bind the buffer to shader access points (bindings)
	createDescriptorSetLayout();
	
	createDescriptorSet();

	//a compute pipeline needs to be created
	//the descriptors created earlier will be submitted here
	createComputePipeline();
	createCommandBuffer();
}
void VulkanTracer::mainLoop()
{
	runCommandBuffer();
	//readDataFromOutputBuffer();
}

void VulkanTracer::cleanup()
{
	
	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
	vkDestroyPipeline(device, pipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	for(int i=0; i<buffers.size();i++){
		vkDestroyBuffer(device, buffers[i], nullptr);
		vkFreeMemory(device, bufferMemories[i], nullptr);
	}
	vkDestroyShaderModule(device, computeShaderModule, nullptr);
	vkDestroyCommandPool(device, commandPool, nullptr);
	vkDestroyDevice(device, nullptr);
	if (enableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}
	vkDestroyInstance(instance, nullptr);
}
void VulkanTracer::createInstance()
{

	//validation layers are used for debugging
	if (enableValidationLayers && !checkValidationLayerSupport())
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}

	//Add description for instance
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "VulkanTracer";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 2, 154);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 2, 154);
	appInfo.apiVersion = VK_API_VERSION_1_2;

	//pointer to description with layer count
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledLayerCount = 0;

	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;

		createInfo.pNext = nullptr;
	}

	//create instance
	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
	if (result != VK_SUCCESS)
		throw std::runtime_error("failed to create instance!");
}
void VulkanTracer::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}
void VulkanTracer::setupDebugMessenger()
{
	if (!enableValidationLayers)
		return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to set up debug messenger!");
	}
}
std::vector<const char *> VulkanTracer::getRequiredExtensions()
{
	std::vector<const char *> extensions;

	if (enableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}
VKAPI_ATTR VkBool32 VKAPI_CALL VulkanTracer::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
{
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

//get vector with available layers
bool VulkanTracer::checkValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	//check all validation layers
	for (const char *layerName : validationLayers)
	{
		bool layerFound = false;

		for (const auto &layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}
		if (!layerFound)
			return false;
	}
	return true;
}

//physical device for computation is chosen
void VulkanTracer::pickPhysicalDevice()
{

	//search for devices
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	if (deviceCount == 0)
		throw std::runtime_error("failed to find GPUs with Vulkan Support!");

	//create vector of devices
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	//search for suitable device for computation task
	for (const auto &device : devices)
	{
		if (isDeviceSuitable(device))
		{
			physicalDevice = device;
			break;
		}
	}
	//error if no device is found
	if (physicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("failed to find suitable GPU!");
	}

	//pick fastest device
	std::multimap<int, VkPhysicalDevice> candidates;
	for (const auto &device : devices)
	{
		candidates.insert(std::make_pair(rateDevice(device), device));
	}
	candidates.rbegin()->first > 0 ? physicalDevice = candidates.rbegin()->second : throw std::runtime_error("failed to find a suitable GPU!");
}

//checks if given device is suitable for computation
//can be extended later if more specific features are needed
bool VulkanTracer::isDeviceSuitable(VkPhysicalDevice device)
{

	//get device properties
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	std::cout << "Found GPU:" << deviceProperties.deviceName << std::endl;

	//get device features
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	QueueFamilyIndices indices = findQueueFamilies(device);
	return indices.hasvalue;
}

//rates devices
int VulkanTracer::rateDevice(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	int score = 1;
	//discrete GPUs are usually faster and get a bonus
	//can be extended to choose the best discrete gpu if multiple are available
	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		score += 1000;
	score += deviceProperties.limits.maxComputeSharedMemorySize;
	return score;
}

VulkanTracer::QueueFamilyIndices VulkanTracer::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;
	indices.hasvalue = 0;
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto &queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			indices.computeFamily = i;
			indices.hasvalue = 1;
		}
		if (indices.isComplete())
			break;
		i++;
	}
	QueueFamily = indices;
	return indices;
}

//creates a logical device to communicate with the physical device
void VulkanTracer::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	//create info about the device queues
	VkDeviceQueueCreateInfo queueCreateInfo{};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = indices.computeFamily;
	queueCreateInfo.queueCount = 1;

	float queuePriority = 1.0f;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	//create info about the device features
	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.shaderFloat64 = true;
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = &queueCreateInfo;
	createInfo.queueCreateInfoCount = 1;
	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = 0;

	//enable validation layers if possible
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(device, indices.computeFamily, 0, &computeQueue);
}

// find memory type with desired properties.
uint32_t VulkanTracer::findMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memoryProperties;

	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

	/*
	How does this search work?
	See the documentation of VkPhysicalDeviceMemoryProperties for a detailed description.
	*/
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		if ((memoryTypeBits & (1 << i)) &&
			((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties))
			return i;
	}
	return -1;
}

void VulkanTracer::createBuffers(){
	//Ray Buffer
	createBuffer(bufferSizes[0], VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffers[0], bufferMemories[0]);
	//output Buffer
	createBuffer(bufferSizes[1], VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffers[1], bufferMemories[1]);
	//Quadric Buffer
	createBuffer(bufferSizes[2], VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, buffers[2], bufferMemories[2]);
	//staging buffer for rays
	createBuffer(bufferSizes[3], VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, buffers[3], bufferMemories[3]);
	//staging buffer for output
	//createBuffer(bufferSizes[3], VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, buffers[4], bufferMemories[4]);
    std::cout << "all buffers created!" << std::endl;
}
void VulkanTracer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory){
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = usage; // buffer is used as a storage buffer.
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;	 // buffer is exclusive to a single queue family at a time.
	VK_CHECK_RESULT(vkCreateBuffer(device, &bufferCreateInfo, NULL, &buffer));
	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

	VkMemoryAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memoryRequirements.size; // specify required memory.
	allocateInfo.memoryTypeIndex = findMemoryType(
		memoryRequirements.memoryTypeBits,properties);
	std::cout << "buffer size: "<<size << std::endl;
	VK_CHECK_RESULT(vkAllocateMemory(device, &allocateInfo, NULL, &bufferMemory)); // allocate memory on device.
	
	// Now associate that allocated memory with the buffer. With that, the buffer is backed by actual memory.
	VK_CHECK_RESULT(vkBindBufferMemory(device, buffer, bufferMemory, 0));
	
    std::cout << "buffer created!" << std::endl;
}
void VulkanTracer::fillRayBuffer(){
	uint32_t bytesNeeded = rayAmount * VULKANTRACER_RAY_DOUBLE_AMOUNT * sizeof(double);
	uint32_t numberOfStagingBuffers = std::ceil((double)bytesNeeded/ (double)bufferSizes[3]); // bufferSizes[3] = 128MB
	std::cout << "Debug Info: number of staging buffers: "<<numberOfStagingBuffers << std::endl;
	std::list<std::vector<Ray>>::iterator raySetIterator;
	raySetIterator = rayList.begin();
	size_t vectorsPerStagingBuffer = std::floor(GPU_MAX_STAGING_SIZE/RAY_VECTOR_SIZE);
	for (int i = 0; i<numberOfStagingBuffers-1; i++){
		fillStagingBuffer(i, raySetIterator, vectorsPerStagingBuffer);
		copyToRayBuffer(i, GPU_MAX_STAGING_SIZE);
		std::cout << "Debug Info: more than 128MB of rays" << std::endl;
	}
	
	std::cout << "numberOfStagingBuffers: "<< numberOfStagingBuffers << ", bytesNeeded: "<<bytesNeeded << std::endl;
	fillStagingBuffer((numberOfStagingBuffers-1)*GPU_MAX_STAGING_SIZE, raySetIterator, vectorsPerStagingBuffer);
	
	std::cout << "Debug Info: fill staging done" << std::endl;
	copyToRayBuffer((numberOfStagingBuffers-1)*GPU_MAX_STAGING_SIZE, ((bytesNeeded-1) % GPU_MAX_STAGING_SIZE)+1);

}
//the input buffer is filled with the ray data
void VulkanTracer::fillStagingBuffer(uint32_t offset, std::list<std::vector<Ray>>::iterator raySetIterator, size_t vectorsPerStagingBuffer)
{
	
	//data is copied to the buffer
	void *data;
	vkMapMemory(device, bufferMemories[3], 0, bufferSizes[3], 0, &data);
	assert((*raySetIterator).size() <= GPU_MAX_STAGING_SIZE);
	//std::cout << "((double)(*raySetIterator).size(): "<<(double)(*raySetIterator).size()<< std::endl;
	vectorsPerStagingBuffer = std::min((size_t)std::ceil(((double)(*raySetIterator).size()*VULKANTRACER_RAY_DOUBLE_AMOUNT*sizeof(double))*rayList.size()*4/GPU_MAX_STAGING_SIZE), vectorsPerStagingBuffer);
	std::cout << "vectorsPerStagingBuffer: "<<vectorsPerStagingBuffer<< std::endl;
	for(int i = 0; i<vectorsPerStagingBuffer; i++){
		memcpy(((char*)data)+i*RAY_VECTOR_SIZE, (*raySetIterator).data(), std::min((*raySetIterator).size()*VULKANTRACER_RAY_DOUBLE_AMOUNT*sizeof(double), (size_t)GPU_MAX_STAGING_SIZE));
		raySetIterator++;
	}
	double* temp = (double*)data;
	//std::cout << "value: "<<temp[0]<< std::endl;
	vkUnmapMemory(device, bufferMemories[3]);
}
void VulkanTracer::copyToRayBuffer(uint32_t offset, uint32_t numberOfBytesToCopy){

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion{};
	copyRegion.dstOffset = offset;
	copyRegion.size = numberOfBytesToCopy;
	vkCmdCopyBuffer(commandBuffer, buffers[3], buffers[0], 1, &copyRegion);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(computeQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(computeQueue);

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);

}
void VulkanTracer::copyToOutputBuffer(uint32_t offset, uint32_t numberOfBytesToCopy){
VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = offset;
	copyRegion.size = numberOfBytesToCopy;
	vkCmdCopyBuffer(commandBuffer, buffers[1], buffers[3], 1, &copyRegion);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(computeQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(computeQueue);

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);


}
void* VulkanTracer::getRays(){
	std::cout << "rayList.size(): "<<rayList.size()<< std::endl;
	std::vector<Ray> data;
	//reserve enough data for all the rays
	/*
    std::cout << "reserving memory"  << std::endl;
	data.reserve((uint64_t)rayAmount * VULKANTRACER_RAY_DOUBLE_AMOUNT);
    std::cout << "reserving memory done"  << std::endl;
	*/
	uint32_t bytesNeeded = rayAmount * VULKANTRACER_RAY_DOUBLE_AMOUNT * sizeof(double);
	uint32_t numberOfStagingBuffers = std::ceil((double)bytesNeeded/ (double)bufferSizes[3]);
	std::cout << "getRays: numberOfStagingBuffers: "<<numberOfStagingBuffers << std::endl;

	//TODO: ONLY FIRST STAGING BUFFER IS TRANSFERED
	numberOfStagingBuffers = 1;

	for (int i = 0; i<numberOfStagingBuffers-1; i++){
		copyToOutputBuffer(i, GPU_MAX_STAGING_SIZE);
		std::cout << "Debug Info: more than 128MB of rays" << std::endl;
		void *mappedMemory = NULL;
		// Map the buffer memory, so that we can read from it on the CPU.
		vkMapMemory(device, bufferMemories[3], 0, GPU_MAX_STAGING_SIZE, 0, &mappedMemory);
		double *pMappedMemory = (double *)mappedMemory;
		// TODO : Currently only the first 16 MB will be transfered to outputData
		for (int j = 0; j < GPU_MAX_STAGING_SIZE/(sizeof(double)*8); j=j+8)
		{
			data.push_back(Ray(pMappedMemory[j],pMappedMemory[j+1],pMappedMemory[j+2],pMappedMemory[j+3],pMappedMemory[j+4],pMappedMemory[j+5], pMappedMemory[j+6]));
		}
		outputData.insertVector(data);
		data.empty();
		vkUnmapMemory(device, bufferMemories[3]);
	}
	std::cout << "numberOfStagingBuffers: "<< numberOfStagingBuffers << ", bytesNeeded: "<<bytesNeeded << std::endl;
	copyToOutputBuffer((numberOfStagingBuffers-1)*GPU_MAX_STAGING_SIZE, ((bytesNeeded-1) % GPU_MAX_STAGING_SIZE)+1);
	void *mappedMemory = NULL;
	// Map the buffer memory, so that we can read from it on the CPU.
	vkMapMemory(device, bufferMemories[3], 0, ((bytesNeeded-1) % GPU_MAX_STAGING_SIZE)+1, 0, &mappedMemory);
	double *pMappedMemory = (double *)mappedMemory;
	// TODO : Currently only the first 16 MB will be transfered to outputData
	for (int j = 0; j < (((bytesNeeded-1) % GPU_MAX_STAGING_SIZE)+1)/(sizeof(double)*8); j=j+8)
	{
		data.push_back(Ray(pMappedMemory[j],pMappedMemory[j+1],pMappedMemory[j+2],pMappedMemory[j+3],pMappedMemory[j+4],pMappedMemory[j+5], pMappedMemory[j+6]));
	}
	std::cout<<"data size= "<< data.size() <<std::endl;
	outputData.insertVector(data);
	data.empty();
	vkUnmapMemory(device, bufferMemories[3]);
    std::cout << "mapping memory done"  << std::endl;
    std::cout << "output size in bytes: "<< (*(outputData.begin())).size() * RAY_DOUBLE_COUNT*sizeof(double)  << std::endl;
	return (*(outputData.begin())).data();
	
}

//the quad buffer is filled with the quadric data
void VulkanTracer::fillQuadricBuffer()
{
    std::cout << "fill quadric buffer" << std::endl;
	//data is copied to the buffer
	void *data;
	vkMapMemory(device, bufferMemories[2], 0, bufferSizes[2], 0, &data);
    std::cout << "map memory done" << std::endl;
    std::cout << "number of quadrics: " << beamline.size()/VULKANTRACER_QUADRIC_DOUBLE_AMOUNT << std::endl;
    std::cout << "size of quadric buffer: " << bufferSizes[2] << std::endl;
	memcpy(data, beamline.data(), bufferSizes[2]);
    std::cout << "memory copy done" << std::endl;
	vkUnmapMemory(device, bufferMemories[2]);

}
void VulkanTracer::createDescriptorSetLayout()
{
	/*
	Here we specify a descriptor set layout. This allows us to bind our descriptors to
	resources in the shader.
	*/

	/*
	Here we specify a binding of type VK_DESCRIPTOR_TYPE_STORAGE_BUFFER to the binding point
	0. This binds to
		layout(std140, binding = 0) buffer ibuf (input)
	and
		layout(std140, binding = 1) buffer obuf (output)
	in the compute shader.
	*/
	//bindings 0, 1 and 2 are used right now
	VkDescriptorSetLayoutBinding descriptorSetLayoutBinding[] = {
		{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL},
		{1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL},
		{2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL}};

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	//2 bindings are used in this layout
	descriptorSetLayoutCreateInfo.bindingCount = 3;
	descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBinding; //TODO

	// Create the descriptor set layout.
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, NULL, &descriptorSetLayout));
}

void VulkanTracer::createDescriptorSet()
{
	/*
	one descriptor for each buffer
	*/
	VkDescriptorPoolSize descriptorPoolSize = {};
	descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorPoolSize.descriptorCount = 3;

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.maxSets = 1; // we need to allocate one descriptor sets from the pool.
	descriptorPoolCreateInfo.poolSizeCount = 1;
	descriptorPoolCreateInfo.pPoolSizes = &descriptorPoolSize;

	// create descriptor pool.
	VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, NULL, &descriptorPool));

	/*
	With the pool allocated, we can now allocate the descriptor set.
	*/
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = descriptorPool; // pool to allocate from.
	descriptorSetAllocateInfo.descriptorSetCount = 1;		   // allocate a single descriptor set.
	descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;

	// allocate descriptor set.
	VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet));

	//no descriptor for the staging buffer
	for(int i=0; i<buffers.size()-2;i++){
	
		//specify which buffer to use: input buffer
		VkDescriptorBufferInfo descriptorBufferInfo = {};
		descriptorBufferInfo.buffer = buffers[i];
		descriptorBufferInfo.offset = 0;
		descriptorBufferInfo.range = bufferSizes[i];

		VkWriteDescriptorSet writeDescriptorSet = {};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = descriptorSet;							   // write to this descriptor set.
		writeDescriptorSet.dstBinding = i;									   // write to the ist binding.
		writeDescriptorSet.descriptorCount = 1;								   // update a single descriptor.
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; // storage buffer.
		writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;

		// perform the update of the descriptor set.
		vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, NULL);
	}
}

// Read file into array of bytes, and cast to uint32_t*, then return.
// The data has been padded, so that it fits into an array uint32_t.
uint32_t *VulkanTracer::readFile(uint32_t &length, const char *filename)
{

	FILE *fp = fopen(filename, "rb");
	if (fp == NULL)
	{
		printf("Could not find or open file: %s\n", filename);
	}

	// get file size.
	fseek(fp, 0, SEEK_END);
	long filesize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	long filesizepadded = long(ceil(filesize / 4.0)) * 4;

	// read file contents.
	char *str = new char[filesizepadded];
	fread(str, filesize, sizeof(char), fp);
	fclose(fp);

	// data padding.
	for (int i = filesize; i < filesizepadded; i++)
	{
		str[i] = 0;
	}

	length = filesizepadded;
	return (uint32_t *)str;
}

void VulkanTracer::createComputePipeline()
{
	/*
	We create a compute pipeline here.
	*/

	/*
	Create a shader module. A shader module basically just encapsulates some shader code.
	*/
	uint32_t filelength;
	// the code in comp.spv was created by running the command:
	// glslangValidator.exe -V shader.comp
	uint32_t *code = readFile(filelength, "comp.spv");
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pCode = code;
	createInfo.codeSize = filelength;

	VK_CHECK_RESULT(vkCreateShaderModule(device, &createInfo, NULL, &computeShaderModule));
	delete[] code;

	/*
	Now let us actually create the compute pipeline.
	It only consists of a single stage with a compute shader.
	So first we specify the compute shader stage, and it's entry point(main).
	*/
	VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {};
	shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	shaderStageCreateInfo.module = computeShaderModule;
	shaderStageCreateInfo.pName = "main";

	/*
	The pipeline layout allows the pipeline to access descriptor sets.
	So we just specify the descriptor set layout we created earlier.
	*/
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
	VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, NULL, &pipelineLayout));

	VkComputePipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stage = shaderStageCreateInfo;
	pipelineCreateInfo.layout = pipelineLayout;

	/*
	Now, we finally create the compute pipeline.
	*/
	VK_CHECK_RESULT(vkCreateComputePipelines(
		device, VK_NULL_HANDLE,
		1, &pipelineCreateInfo,
		NULL, &pipeline));
}

void VulkanTracer::createCommandPool(){
	/*
	In order to send commands to the device(GPU),
	we must first record commands into a command buffer.
	To allocate a command buffer, we must first create a command pool. So let us do that.
	*/
	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.flags = 0;
	// the queue family of this command pool. All command buffers allocated from this command pool,
	// must be submitted to queues of this family ONLY.
	commandPoolCreateInfo.queueFamilyIndex = QueueFamily.computeFamily;
	VK_CHECK_RESULT(vkCreateCommandPool(device, &commandPoolCreateInfo, NULL, &commandPool));
}

void VulkanTracer::createCommandBuffer()
{
	/*
	Now allocate a command buffer from the command pool.
	*/
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = commandPool; // specify the command pool to allocate from.
	// if the command buffer is primary, it can be directly submitted to queues.
	// A secondary buffer has to be called from some primary command buffer, and cannot be directly
	// submitted to a queue. To keep things simple, we use a primary command buffer.
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = 1;											   // allocate a single command buffer.
	VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer)); // allocate command buffer.

	/*
	Now we shall start recording commands into the newly allocated command buffer.
	*/
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;	  // the buffer is only submitted and used once in this application.
	VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo)); // start recording commands.

	/*
	We need to bind a pipeline, AND a descriptor set before we dispatch.
	The validation layer will NOT give warnings if you forget these, so be very careful not to forget them.
	*/
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);
	//vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSets[1], 0, NULL);
	/*
	Calling vkCmdDispatch basically starts the compute pipeline, and executes the compute shader.
	The number of workgroups is specified in the arguments.
	If you are already familiar with compute shaders from OpenGL, this should be nothing new to you.
	*/
	vkCmdDispatch(commandBuffer, (uint32_t)ceil(rayAmount / float(WORKGROUP_SIZE)), 1, 1);

	VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer)); // end recording commands.
}

void VulkanTracer::runCommandBuffer()
{
	/*
	Now we shall finally submit the recorded command buffer to a queue.
	*/

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;			 // submit a single command buffer
	submitInfo.pCommandBuffers = &commandBuffer; // the command buffer to submit.

	/*
		We create a fence.
	*/
	VkFence fence;
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = 0;
	VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo, NULL, &fence));

	/*
	We submit the command buffer on the queue, at the same time giving a fence.
	*/
	VK_CHECK_RESULT(vkQueueSubmit(computeQueue, 1, &submitInfo, fence));
	/*
	The command will not have finished executing until the fence is signalled.
	So we wait here.
	We will directly after this read our buffer from the GPU,
	and we will not be sure that the command has finished executing unless we wait for the fence.
	Hence, we use a fence here.
	*/
	VK_CHECK_RESULT(vkWaitForFences(device, 1, &fence, VK_TRUE, 100000000000));

	vkDestroyFence(device, fence, NULL);
}
void VulkanTracer::setRayAmount(uint32_t inputRayAmount)
{
	rayAmount = inputRayAmount;
}
void VulkanTracer::setRayAmount()
{
	rayAmount = rayList.rayAmount();
}

void VulkanTracer::addRayVector(void* location, size_t size){
	std::vector<Ray> newRayVector;
    std::cout<<"1"<<std::endl;
	newRayVector.resize(size);
    //std::cout<<"2"<<std::endl;	
    //std::cout<<"addRayVector: size= "<<size<<std::endl;
	memcpy(&newRayVector[0], location, size * VULKANTRACER_RAY_DOUBLE_AMOUNT * sizeof(double));
    //std::cout<<"3 "<< newRayVector.size() << std::endl;
	rayList.insertVector(newRayVector);
    std::cout<<"4"<<std::endl;

}
//adds quad to beamline
void VulkanTracer::addQuadric(std::vector<double> inQuadric, std::vector<double> inputInMatrix, std::vector<double> inputOutMatrix, std::vector<double> misalignmentMatrix, std::vector<double> inverseMisalignmentMatrix){
	assert(inQuadric.size() == 16 && inputInMatrix.size() == 16 && inputOutMatrix.size() == 16 && misalignmentMatrix.size() == 16);
	//beamline.resize(beamline.size()+1);
	beamline.insert(beamline.end(), inQuadric.begin(), inQuadric.end());
	beamline.insert(beamline.end(), inputInMatrix.begin(), inputInMatrix.end());
	beamline.insert(beamline.end(), inputOutMatrix.begin(), inputOutMatrix.end());
	beamline.insert(beamline.end(), misalignmentMatrix.begin(), misalignmentMatrix.end());
	beamline.insert(beamline.end(), inverseMisalignmentMatrix.begin(), inverseMisalignmentMatrix.end());
}
void VulkanTracer::divideAndSortRays(){
	for(auto i = rayList.begin(); i!=rayList.end(); i++){
		
	}
}

//is not used anymore
int VulkanTracer::main()
{
	VulkanTracer app;

	try
	{
		app.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		std::cout << "finished VulkanTracer failure" << std::endl;
		return EXIT_FAILURE;
	}
	std::cout << "finished VulkanTracer2" << std::endl;
	return EXIT_SUCCESS;
}
