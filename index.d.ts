declare module "tolw" {
  /**
   * The parsed geometry data
   */
  export interface IGeometryData {
    /**
     * The vertices of the geometry
     */
    vertices: Float32Array;
    /**
     * The normals of the geometry
     */
    normals: Float32Array;
    /**
     * The uvs of the geometry
     */
    uvs: Float32Array;
    /**
     * The indices of the geometry
     */
    indices: Uint32Array;
  }

  /**
   * Initializes the wasm module
   */
  export function init(): Promise<boolean>;

  /**
   * Parses the provided obj binary data
   * @param data - The obj binary data
   */
  export function init(data: (ArrayBuffer | ArrayBufferView)): IGeometryData;

}
