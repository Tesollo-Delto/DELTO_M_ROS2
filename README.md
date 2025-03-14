# DELTO_M_ROS2 

## 📌 Overview

<img src="/dg_description/image/dg5f_1.png" alt="dg5f" width="400px"/>

The DELTO_M_ROS2 Repository is a comprehensive ROS 2 package designed to support the DG-M. This project includes simulations, control interfaces, and visualization tools for the gripper, enabling developers to efficiently develop and test robotic applications.

---

## 📌 **Supported ROS Distributions**

|  **ROS Version** |  **Ubuntu Version** |  **Branch** |
|------------------|----------------------|---------------|
| ROS 2            | 22.04 (Jammy)        | `humble`      |


## 📦 Package Structure

The DELTO_M_ROS2 project comprises the following main packages:

- [**dg5f_gz**](/dg5f_gz/): Provides Gazebo simulations and control interfaces for the DG5F gripper. 
- [**dg_description**](/dg_description/): Contains URDF models and visualization configurations for the DG5F gripper.

---

## 🛠️ Installation and Build Instructions

To install and build the DELTO_M_ROS2 project, follow these steps:

1. **Create Workspace and Clone Source Code**

   ```bash
   mkdir -p ~/your_ws/src
   cd ~/your_ws/src
   git clone <DELTO_M_ROS2_repository_URL>
   ```

2. **Install Dependencies**

   ```bash
   cd ~/delto_m_ws
   rosdep install --from-paths src --ignore-src -r -y
   ```

3. **Build Packages**

   ```bash
   colcon build
   ```

4. **Source the Environment**

   ```bash
   source install/setup.bash
   ```

---

## 🎯 **Performance Demonstrations**

### **DG-5F Robot Hand: Plugging in an Ethernet Cable**

[![DG-5F Ethernet Cable Plugging](https://img.youtube.com/vi/pi-aJvU8Jug/sddefault.jpg)](https://www.youtube.com/watch?v=pi-aJvU8Jug)  
▶️ *Click the image to watch the video*

---

### **DG-5F Robot Hand: Paper Cup Removal Demonstration**

[![DG-5F Paper Cup Removal](https://img.youtube.com/vi/MlUSlto5R9U/sddefault.jpg)](https://www.youtube.com/watch?v=MlUSlto5R9U)  
▶️ *Click the image to watch the video*

## 🤝 Contributing

The DELTO_M_ROS2 project is open-source, and contributions are welcome. To contribute:

1. Fork this repository.
2. Create a new branch (`git checkout -b feature/my-feature`).
3. Commit your changes (`git commit -am 'Add my feature'`).
4. Push to your branch (`git push origin feature/my-feature`).
5. Open a pull request detailing your modifications.

---

## 📄 License

This project is released under the BSD-3-Clause license.

---

## 📧 Contact

For additional support or inquiries about this project, please contact [TESOLLO SUPPORT](mailto:support@tesollo.com). 