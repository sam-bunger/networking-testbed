import React, { useEffect, useRef, useState } from "react";
import uPlot from "uplot";
import { MAIN } from "../../wasm";

interface SimulationComponentProps {
  simulation: MAIN.ClientSimulationController;
}

export const ClientSimulationStats = ({
  simulation,
}: SimulationComponentProps) => {
  const dataRef = useRef<{
    predictedFrameCount: number[];
    downloadKbps: number[];
    uploadKbps: number[];
  }>({
    predictedFrameCount: Array(60).fill(0),
    downloadKbps: Array(60).fill(0),
    uploadKbps: Array(60).fill(0),
  });

  const frameCountPlotRef = useRef<uPlot>();
  const downloadPlotRef = useRef<uPlot>();
  const frameCountContainerRef = useRef<HTMLDivElement>(null);
  const downloadContainerRef = useRef<HTMLDivElement>(null);

  const [currentFrameCount, setCurrentFrameCount] = useState(0);
  const [currentDownloadKbps, setCurrentDownloadKbps] = useState(0);
  const [currentUploadKbps, setCurrentUploadKbps] = useState(0);

  useEffect(() => {
    if (!frameCountContainerRef.current || !downloadContainerRef.current)
      return;

    const xValues = Array(60)
      .fill(0)
      .map((_, i) => i);

    const frameCountPlot = new uPlot(
      {
        width: frameCountContainerRef.current.clientWidth,
        height: 100,
        series: [
          {},
          {
            stroke: "#4CAF50",
            fill: "transparent",
            width: 2,
          },
        ],
        axes: [{ show: false }],
        scales: {
          x: {
            time: false,
          },
          y: {
            range: (u, min, max) => [0, Math.max(max * 1.1, 50)],
          },
        },
        legend: {
          show: false,
        },
        padding: [10, 0, 10, 0],
      },
      [xValues, dataRef.current.predictedFrameCount],
      frameCountContainerRef.current
    );

    const downloadPlot = new uPlot(
      {
        width: downloadContainerRef.current.clientWidth,
        height: 100,
        series: [
          {},
          {
            stroke: "#2196F3",
            fill: "transparent",
            width: 2,
            label: "Download",
          },
          {
            stroke: "#FF4081",
            fill: "transparent",
            width: 2,
            label: "Upload",
          },
        ],
        axes: [{ show: false }],
        scales: {
          x: {
            time: false,
          },
          y: {
            range: (u, min, max) => [0, Math.max(max * 1.1, 10)],
          },
        },
        legend: {
          show: false,
        },
        padding: [10, 0, 10, 0],
      },
      [xValues, dataRef.current.downloadKbps, dataRef.current.uploadKbps],
      downloadContainerRef.current
    );

    frameCountPlotRef.current = frameCountPlot;
    downloadPlotRef.current = downloadPlot;

    return () => {
      frameCountPlot.destroy();
      downloadPlot.destroy();
    };
  }, []);

  useEffect(() => {
    const interval = setInterval(() => {
      const predictedFrameCount = simulation.getLastPredictedFrameCount();
      const downloadKbps = simulation.getDownloadBps() / 1000;
      const uploadKbps = simulation.getUploadBps() / 1000;

      dataRef.current.predictedFrameCount = [
        ...dataRef.current.predictedFrameCount.slice(1),
        predictedFrameCount,
      ];
      dataRef.current.downloadKbps = [
        ...dataRef.current.downloadKbps.slice(1),
        downloadKbps,
      ];
      dataRef.current.uploadKbps = [
        ...dataRef.current.uploadKbps.slice(1),
        uploadKbps,
      ];

      setCurrentFrameCount(predictedFrameCount);
      setCurrentDownloadKbps(downloadKbps);
      setCurrentUploadKbps(uploadKbps);

      if (frameCountPlotRef.current && downloadPlotRef.current) {
        frameCountPlotRef.current.setData([
          Array(60)
            .fill(0)
            .map((_, i) => i),
          dataRef.current.predictedFrameCount,
        ]);
        downloadPlotRef.current.setData([
          Array(60)
            .fill(0)
            .map((_, i) => i),
          dataRef.current.downloadKbps,
          dataRef.current.uploadKbps,
        ]);
      }
    }, 1000 / 60);

    return () => clearInterval(interval);
  }, [simulation]);

  return (
    <div style={{ width: "100%" }}>
      <h3>Predicted Frame Count</h3>
      <div style={{ display: "flex", alignItems: "center", gap: "8px" }}>
        <div
          ref={frameCountContainerRef}
          style={{ height: "100px", flex: 1 }}
        />
        <div style={{ fontSize: "14px", minWidth: "60px" }}>
          {currentFrameCount}
        </div>
      </div>

      <h3>Network Throughput (Kbps)</h3>
      <div style={{ display: "flex", alignItems: "center", gap: "8px" }}>
        <div ref={downloadContainerRef} style={{ height: "100px", flex: 1 }} />
        <div style={{ fontSize: "14px", minWidth: "100px" }}>
          <div style={{ color: "#2196F3" }}>↓ {currentDownloadKbps}</div>
          <div style={{ color: "#FF4081" }}>↑ {currentUploadKbps}</div>
        </div>
      </div>
    </div>
  );
};
