#include "ScoobzEngine.h"


namespace ScoobzEngine{
	ScoobzEngine::ScoobzEngine(){}
	// anytime you use 'new' you gotta delete the object...
	ScoobzEngine::~ScoobzEngine(){
		delete swapchain;
		delete graphicsCommandPool;
		delete transferCommandPool;
		delete vertexBuffer;
	}
	// public function to call private functions...
	void ScoobzEngine::Initvulkan(){
		InitWindow();
		CreateInstance(); 
		SetupDebugCallback();
		CreateSurface();
		GetPhysicalDevices();
		CreateLogicalDevice();
		swapchain = new SESwapChain(); // create instance of the swapchain so we can use it in the engine...
		swapchain->Create(&logicalDevice, &physicalDevice, &surface, &windowObj, QuerySwapChainSupport(physicalDevice));//create the swapchain
		CreateRenderPass();
		CreateGraphicsPipeline();
		swapchain->CreateFramebuffers(&renderPass); //create swapchain frame buffers, pass in renderpass information..

		QueueFamilyIndices indices = FindQueueFamilies(&physicalDevice, &surface);
		graphicsCommandPool = new SECommandPool(&logicalDevice, indices.graphicsFamily);
		transferCommandPool = new SECommandPool(&logicalDevice, indices.transferFamily, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);

		vertexBuffer = new SEVertexBuffer(&logicalDevice, &physicalDevice, &surface, transferCommandPool->GetCommandPool(), &transferQueue);
		CreateCommandBuffers();
		CreateSemaphores();
	}

	void ScoobzEngine::CleanupVulkan(){
		CleanupSwapChain();

		vertexBuffer->Cleanup(&logicalDevice);

		vertexBuffer->Cleanup(&logicalDevice);

		vkDestroySemaphore(logicalDevice, renderFinishedSemaphore, VK_NULL_HANDLE);
		vkDestroySemaphore(logicalDevice, imageAvailableSemaphore, VK_NULL_HANDLE);

		graphicsCommandPool->Cleanup(&logicalDevice);
		transferCommandPool->Cleanup(&logicalDevice);

		vkDestroyDevice(logicalDevice, VK_NULL_HANDLE);
		DestroyDebugReportCallbackEXT(instance, callback, VK_NULL_HANDLE);
		vkDestroySurfaceKHR(instance, surface, VK_NULL_HANDLE);
		vkDestroyInstance(instance, VK_NULL_HANDLE);
		glfwDestroyWindow(windowObj.window);
		glfwTerminate();
	}

	void ScoobzEngine::CleanupSwapChain(){
		swapchain->CleanupFramebuffers();

		vkFreeCommandBuffers(logicalDevice, *graphicsCommandPool->GetCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

		vkDestroyPipeline(logicalDevice, graphicsPipeline, VK_NULL_HANDLE);
		vkDestroyPipelineLayout(logicalDevice, pipelineLayout, VK_NULL_HANDLE);
		vkDestroyRenderPass(logicalDevice, renderPass, VK_NULL_HANDLE);

		swapchain->Cleanup();
	}

	void ScoobzEngine::GameLoop(){
		while (!glfwWindowShouldClose(windowObj.window)){
			glfwPollEvents();

			DrawFrame();
		}

		vkDeviceWaitIdle(logicalDevice);
		CleanupVulkan();
	}

	void ScoobzEngine::InitWindow(){
		windowObj.Create();
		glfwSetWindowUserPointer(windowObj.window, this);
		glfwSetWindowSizeCallback(windowObj.window, ScoobzEngine::OnWindowResized);
	}

	void ScoobzEngine::RecreateSwapChain(){
		vkDeviceWaitIdle(logicalDevice);
		CleanupSwapChain();

		swapchain->Create(&logicalDevice, &physicalDevice, &surface, &windowObj, QuerySwapChainSupport(physicalDevice));
		CreateRenderPass();
		CreateGraphicsPipeline();
		swapchain->CreateFramebuffers(&renderPass);
		CreateCommandBuffers();
	}

	void ScoobzEngine::CreateInstance(){
		//check for validation layer availablity
		if (enableValidationLayers && !CheckValidationLayerSupport()){ //if the check fails, we throw an error and exit.
			throw std::runtime_error("Validation layers requested but are not available");
		}
		else{
			std::cout << "Requested Layers Availability: SUCCESSFUL!" << std::endl;
		}
		// application info struct..
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; //Structure type for application info
		appInfo.pNext = nullptr; // pNext is a pointer to the linked list of structures.. dont need it
		appInfo.pApplicationName = "ScoobzEngine"; // name of your application
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); // version of your app, major,minor,patch
		appInfo.pEngineName = "ScoobzEngine"; // name of your engine
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0); // engine version, majore,minor,patch
		appInfo.apiVersion = VK_API_VERSION_1_0; // this is the minimum version of Vulkan allowed to run the application 

		//Create info struct. tells the instance all its information needed to be created
		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; // sType is the structure type Vulkan has them prebuilt.
		createInfo.pNext = nullptr; // pNext is a pointer to the linked list of structures.. dont need it
		createInfo.flags = 0; // were not using any flags
		createInfo.pApplicationInfo = &appInfo; // this is a pointer to the app info struct.
		//check if validatoin layers is enabled
		if (enableValidationLayers){
			createInfo.enabledLayerCount = validationLayers.size(); //if validation layers then set the layer count to validation layers size
			createInfo.ppEnabledLayerNames = validationLayers.data(); //set layer names to the data in the layer vector.
		}
		else{
			createInfo.enabledLayerCount = 0; // if validation layers isnt on, then set the layer count to 0
			createInfo.ppEnabledLayerNames = nullptr; // and set the layer names to nullptr.
		}
		// since were using glfw we have to use some extensions so Vulkan can interface with it.
		auto extensions = GetRequiredExtensions();
		createInfo.enabledExtensionCount = extensions.size(); // get the count from the extensions vector size
		createInfo.ppEnabledExtensionNames = extensions.data(); // get the data from the vector...

		// this creates the instace, using the createInfo struct, the allocator is nullptr so vulkan will use its
		// own built in one, and then we pass a pointer to the instance member variable.
		// check if VK_success
		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS){
			throw std::runtime_error("Failed to create an instance!"); // throw runtime error if we fail to create instance
		}
		else{
			std::cout << "Vulkan Instance Creation: SUCCESSFUL!" << std::endl; // tell me if we succeeded 'this isnt needed'
		}
	}

	bool ScoobzEngine::CheckValidationLayerSupport(){
		uint32_t layerCount; //Vulkan prefers unsigned integer at 32 bits universal type...
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr); // set this to a nullptr first, then we set the size after

		std::vector<VkLayerProperties> availableLayers(layerCount); // vector of the layer properties
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()); // now we set the data from the vector into this
		//iterate through the validation layers to make sure all of them are within the layers
		for (const char* layerName : validationLayers){
			bool layerFound = false; // asume its not found
			for (const auto &layerProperties : availableLayers){
				if (strcmp(layerName, layerProperties.layerName) == 0){ // string compare layer name and properties name
					layerFound = true; // if they match, then set it to found and break cause we got what we need.
					break;
				}
			}
			if (!layerFound){
				return false; // else if we dont find a layer we return false.
			}
		}
		return true;
	}

	std::vector<const char*> ScoobzEngine::GetRequiredExtensions(){
		std::vector<const char*> extensions; // vector of extensions

		unsigned int glfwExtensionCount = 0; // set it to 0
		const char** glfwExtensions; 
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount); // call a built in function that takes the count

		for (unsigned int i = 0; i < glfwExtensionCount; i++){ // iterate through and add to the vector..
			extensions.push_back(glfwExtensions[i]);
		}

		if (enableValidationLayers){
			extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME); // if we have validation layers then add this to the end of each extension
		}

		return extensions; // then return the extensions vector...
	}

	void ScoobzEngine::SetupDebugCallback(){
		if (!enableValidationLayers){
			return;
		}

		VkDebugReportCallbackCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		createInfo.pfnCallback = debugCallback;

		if (CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback) != VK_SUCCESS){
			throw std::runtime_error("Failed to set up debug callback");
		}
		else{
			std::cout << "Debug Callback Setup: SUCCESSFUL!" << std::endl;
		}
	}

	void ScoobzEngine::CreateSurface(){
		if (glfwCreateWindowSurface(instance, windowObj.window, nullptr, &surface) != VK_SUCCESS){
			throw std::runtime_error("Failed to create window surface");
		}
		else{
			std::cout << "Window Surface Creation: SUCCESSFUL!" << std::endl;
		}
	}

	void ScoobzEngine::GetPhysicalDevices(){
		uint32_t physicalDeviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
		if (physicalDeviceCount == 0){
			throw std::runtime_error("No devices found with vulkan support");
		}

		std::vector<VkPhysicalDevice> foundPhysicalDevices(physicalDeviceCount);
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, foundPhysicalDevices.data());

		std::multimap<int, VkPhysicalDevice> rankedDevices;

		for (const auto& currentDevice : foundPhysicalDevices){
			int score = RateDeviceSuitability(currentDevice);
			rankedDevices.insert(std::make_pair(score, currentDevice));
		}

		if (rankedDevices.rbegin()->first > 0){
			physicalDevice = rankedDevices.rbegin()->second;
			std::cout << "Physical Device found: SUCCESSFUL!" << std::endl;
		}
		else{
			throw std::runtime_error("No physical devices meet necessary criteria");
		}
	}

	SwapChainSupportDetails ScoobzEngine::QuerySwapChainSupport(VkPhysicalDevice device){
		SwapChainSupportDetails details;

		// capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		//formats
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0){
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		//presentModes
		uint32_t presentModesCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModesCount, nullptr);
		if (presentModesCount != 0){
			details.presentModes.resize(presentModesCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModesCount, details.presentModes.data());
		}

		return details;
	}

	int ScoobzEngine::RateDeviceSuitability(VkPhysicalDevice deviceToRate){
		int score = 0;

		QueueFamilyIndices indices = FindQueueFamilies(&deviceToRate, &surface);
		bool extensionsSupported = CheckDeviceExtensionSupport(deviceToRate);
		if (!indices.isComplete() || !extensionsSupported){
			return 0;
		}

		bool swapChainAdequate = false;
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(deviceToRate);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		if (!swapChainAdequate){
			return 0;
		}


		VkPhysicalDeviceFeatures deviceFeatures;
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(deviceToRate, &deviceProperties);
		vkGetPhysicalDeviceFeatures(deviceToRate, &deviceFeatures);

		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
			score += 1000;
		}

		score += deviceProperties.limits.maxImageDimension2D;

		if (!deviceFeatures.geometryShader){
			return 0;
		}

		return score;
	}

	void ScoobzEngine::CreateLogicalDevice(){
		QueueFamilyIndices indices = FindQueueFamilies(&physicalDevice, &surface);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<int> uniqueQueueFamilies = { indices.graphicsFamily, indices.transferFamily };
		const float queuePriority = 1.0f;
		for (int queueFamily : uniqueQueueFamilies){
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.pNext = nullptr;
			queueCreateInfo.flags = 0;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures = {};

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.queueCreateInfoCount = 1;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		if (enableValidationLayers){
			createInfo.enabledLayerCount = validationLayers.size();
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else{
			createInfo.enabledLayerCount = 0;
			createInfo.ppEnabledLayerNames = nullptr;
		}
		createInfo.enabledExtensionCount = deviceExtensions.size();
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();
		createInfo.pEnabledFeatures = &deviceFeatures;

		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS){
			throw std::runtime_error("Failed to create logical device");
		}
		else{
			std::cout << "Logical device created successfully" << std::endl;
		}

		vkGetDeviceQueue(logicalDevice, indices.graphicsFamily, 0, &graphicsQueue);
		vkGetDeviceQueue(logicalDevice, indices.transferFamily, 0, &transferQueue);
	}

	void ScoobzEngine::CreateRenderPass(){
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = *swapchain->GetImageFormat();
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS){
			throw std::runtime_error("Failed to create Render pass");
		}
		else{
			std::cout << "Render Pass Creation: SUCCESSFUL!" << std::endl;
		}
	}

	void ScoobzEngine::CreateGraphicsPipeline(){
		auto vertShaderCode = ReadFile("Shaders/vert.spv");
		auto fragShaderCode = ReadFile("Shaders/frag.spv");

		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;

		vertShaderModule = CreateShaderModule(vertShaderCode);
		fragShaderModule = CreateShaderModule(fragShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = false;

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapchain->GetExtent()->width;
		viewport.height = (float)swapchain->GetExtent()->height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0,0 };
		scissor.extent = *swapchain->GetExtent();

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;

		VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH };

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 2;
		dynamicState.pDynamicStates = dynamicStates;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS){
			throw std::runtime_error("Failed to create pipeline layout");
		}
		else{
			std::cout << "Pipeline Layout Creation: SUCCESSFUL!" << std::endl;
		}

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;


		if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS){
			throw std::runtime_error("Failed to create Graphics pipeline");
		}
		else{
			std::cout << "Graphics Pipeline Creation: SUCCESSFUL!" << std::endl;
		}

		vkDestroyShaderModule(logicalDevice, vertShaderModule, VK_NULL_HANDLE);
		vkDestroyShaderModule(logicalDevice, fragShaderModule, VK_NULL_HANDLE);
	}

	VkShaderModule ScoobzEngine::CreateShaderModule(const std::vector<char>& code){

		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();

		std::vector<uint32_t> codeAligned(code.size() / sizeof(uint32_t) + 1);
		memcpy(codeAligned.data(), code.data(), code.size());
		createInfo.pCode = codeAligned.data();

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS){
			throw std::runtime_error("Failed to create shader module");
		}
		else{
			std::cout << "Shader Module Creation: SUCCESSFUL!" << std::endl;
		}

		return shaderModule;
	}

	void ScoobzEngine::CreateCommandBuffers(){
		commandBuffers.resize(swapchain->GetFramebufferSize());

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = *graphicsCommandPool->GetCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS){
			throw std::runtime_error("Failed to create Command Buffer");
		}
		else{
			std::cout << "Command Buffers Creation: SUCCESSFUL!" << std::endl;
		}

		for (size_t i = 0; i < commandBuffers.size(); i++){
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; //qeues command

			vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderPass;
			renderPassInfo.framebuffer = *swapchain->GetFramebuffer(i);
			renderPassInfo.renderArea.offset = { 0,0 };
			renderPassInfo.renderArea.extent = *swapchain->GetExtent();
			VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

			VkBuffer vertexBuffers[] = { *vertexBuffer->GetBuffer() };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

			VkBuffer indexBuffer = *vertexBuffer->GetIndexBuffer();
			vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(vertexBuffer->GetIndicesSize()), 1, 0, 0, 0);

			vkCmdEndRenderPass(commandBuffers[i]);

			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS){
				throw std::runtime_error("Failed to record command buffer");
			}
		}
	}

	void ScoobzEngine::CreateSemaphores(){

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS
			|| vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS){
			throw std::runtime_error("Failed to Create Semaphores");
		}
		else{
			std::cout << "Semaphores Creation: SUCCESSFUL!" << std::endl;
		}
	}

	void ScoobzEngine::DrawFrame(){
		uint32_t imageIndex;
		vkAcquireNextImageKHR(logicalDevice, *swapchain->GetSwapchain(), std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS){
			throw std::runtime_error("Failed to submit draw command buffer");
		}

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapchains[] = { *swapchain->GetSwapchain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapchains;
		presentInfo.pImageIndices = &imageIndex;

		vkQueuePresentKHR(graphicsQueue, &presentInfo);
	}

	bool ScoobzEngine::CheckDeviceExtensionSupport(VkPhysicalDevice device){
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		for (const char* currentExtension : deviceExtensions){
			bool extensionFound = false;
			for (const auto& extension : availableExtensions){
				if (strcmp(currentExtension, extension.extensionName) == 0){
					extensionFound = true;
					break;
				}
			}
			if (!extensionFound){
				return false;
			}
		}

		return true;
	}

	void ScoobzEngine::OnWindowResized(GLFWwindow* window, int width, int height) {
		if (width == 0 || height == 0) { 
			return; 
		}
		ScoobzEngine* engine = reinterpret_cast<ScoobzEngine*>(glfwGetWindowUserPointer(window));
		engine->RecreateSwapChain();
	}

	VkResult ScoobzEngine::CreateDebugReportCallbackEXT(
		VkInstance instance, 
		const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator, 
		VkDebugReportCallbackEXT* pCallback) {
		auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pCallback);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void ScoobzEngine::DestroyDebugReportCallbackEXT(
		VkInstance instance,
		VkDebugReportCallbackEXT callback,
		const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
		if (func != nullptr) {
			func(instance, callback, pAllocator);
		}
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL ScoobzEngine::debugCallback(
		VkDebugReportFlagsEXT flags, // holds the message being output
		VkDebugReportObjectTypeEXT objType, // descirbes which object the message is comeing from
		uint64_t obj, // describes the object type...
		size_t location, 
		int32_t code, 
		const char* layerPrefix,
		const char* msg, // this is a pointer to the error message being output
		void* userData) {
		// this is used in case there is any errors during validation layers. this will output the message so you know whats wrong...
		std::cerr << "Validation layer: " << msg << std::endl;
		return VK_FALSE;
	}

}