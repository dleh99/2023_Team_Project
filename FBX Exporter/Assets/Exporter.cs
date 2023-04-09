using System.Collections;
using System.Collections.Generic;
using System.IO;
using Unity.VisualScripting;
using UnityEditor.Timeline;
using UnityEngine;

public class Exporter : MonoBehaviour
{
    StreamWriter writer = null;
    BinaryWriter binaryWriter = null;

    // Start is called before the first frame update
    void Start()
    {
        MeshFilter[] modelMeshFilters = this.gameObject.GetComponentsInChildren<MeshFilter>();
        Transform[] transforms = new Transform[modelMeshFilters.Length];
        for (int i = 0; i < modelMeshFilters.Length; ++i)
        {
            transforms[i] = modelMeshFilters[i].GetComponent<Transform>();
        }


        // text file
        writer = File.CreateText("player model data.txt");

        for (int i=0; i < modelMeshFilters.Length; ++i)
        {
            int verticesLength = modelMeshFilters[i].mesh.vertices.Length;
            int normalsLength = modelMeshFilters[i].mesh.normals.Length;
            int tangentsLength = modelMeshFilters[i].mesh.tangents.Length;
            int uv0Length = modelMeshFilters[i].mesh.uv.Length;

            writer.WriteLine(modelMeshFilters[i].name);
            
            writer.WriteLine("Vertices");
            writer.WriteLine(verticesLength);
            for(int j=0;j< verticesLength; ++j)
            {
                writer.Write(modelMeshFilters[i].mesh.vertices[j]);
            }
            writer.WriteLine('\n');

            writer.WriteLine("Normals");
            writer.WriteLine(normalsLength);
            for (int j = 0; j < normalsLength; ++j)
            {
                writer.Write(modelMeshFilters[i].mesh.normals[j]);
            }
            writer.WriteLine('\n');

            writer.WriteLine("Tangents");
            writer.WriteLine(tangentsLength);
            for (int j = 0; j < tangentsLength; ++j)
            {
                writer.Write(modelMeshFilters[i].mesh.tangents[j]);
            }
            writer.WriteLine('\n');

            writer.WriteLine("Uv0");
            writer.WriteLine(uv0Length);
            for (int j = 0; j < uv0Length; ++j)
            {
                writer.Write(modelMeshFilters[i].mesh.uv[j]);
            }
            writer.Write('\n');
            writer.Write("#########################");
            writer.WriteLine('\n');
        }

        // binary file
        binaryWriter = new BinaryWriter(File.Open("player model data.bin", FileMode.Create));

        for (int i = 0; i < modelMeshFilters.Length; ++i)
        {
            int verticesLength = modelMeshFilters[i].mesh.vertices.Length;
            int normalsLength = modelMeshFilters[i].mesh.normals.Length;
            int tangentsLength = modelMeshFilters[i].mesh.tangents.Length;
            int uv0Length = modelMeshFilters[i].mesh.uv.Length;

            binaryWriter.Write(verticesLength);
            binaryWriter.Write("*");
            for (int j = 0; j < verticesLength; ++j)
            {
                binaryWriter.Write(modelMeshFilters[i].mesh.vertices[j].x);
                binaryWriter.Write(modelMeshFilters[i].mesh.vertices[j].y);
                binaryWriter.Write(modelMeshFilters[i].mesh.vertices[j].z);
            }
            binaryWriter.Write("*");

            binaryWriter.Write(normalsLength);
            binaryWriter.Write("*");
            for (int j = 0; j < normalsLength; ++j)
            {
                binaryWriter.Write(modelMeshFilters[i].mesh.normals[j].x);
                binaryWriter.Write(modelMeshFilters[i].mesh.normals[j].y);
                binaryWriter.Write(modelMeshFilters[i].mesh.normals[j].z);
            }
            binaryWriter.Write("*");

            binaryWriter.Write(tangentsLength);
            binaryWriter.Write("*");
            for (int j = 0; j < tangentsLength; ++j)
            {
                binaryWriter.Write(modelMeshFilters[i].mesh.tangents[j].x);
                binaryWriter.Write(modelMeshFilters[i].mesh.tangents[j].y);
                binaryWriter.Write(modelMeshFilters[i].mesh.tangents[j].z);
            }
            binaryWriter.Write("*");

            binaryWriter.Write(uv0Length);
            binaryWriter.Write("*");
            for (int j = 0; j < uv0Length; ++j)
            {
                binaryWriter.Write(modelMeshFilters[i].mesh.uv[j].x);
                binaryWriter.Write(modelMeshFilters[i].mesh.uv[j].y);
            }
            binaryWriter.Write("*");
        }

        binaryWriter.Close();
    }

    // Update is called once per frame
    void Update()
    {
        
    }
}
