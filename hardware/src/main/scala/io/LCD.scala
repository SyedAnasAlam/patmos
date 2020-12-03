package io

import chisel3._
import ocp._

object LCD extends DeviceObject {
    def init(params: Map[String, String]) = {}

    def create(params: Map[String, String]): LCD = Module(new LCD())

 //   trait Pins {}
}

class LCD extends CoreDevice() {
    override val io = IO(new CoreDeviceIO with patmos.HasPins {
        override val pins = new Bundle() {
            val lcdData = Output(Vec(8, UInt(1.W)))
            val lcdEn = Output(Bool())
            val lcdRW = Output(Bool())
            val lcdRS = Output(Bool())
            val lcdOn = Output(Bool())
        }
    })
    //Default
    val dataReg = RegInit(0.U(12.W))

    //Default response
    val respReg = RegInit(OcpResp.NULL)
    respReg := OcpResp.NULL

    val masterReg = RegNext(io.ocp.M)

    when(masterReg.Cmd === OcpCmd.WR) {
        dataReg := masterReg.Data(12, 0)
    }

    when(masterReg.Cmd === OcpCmd.WR) {
        respReg := OcpResp.DVA
    }

    io.ocp.S.Resp := respReg

    io.pins.lcdData := dataReg(7, 0)
    io.pins.lcdEn := dataReg(8)
    io.pins.lcdRW := dataReg(9)
    io.pins.lcdRS := dataReg(10)
    io.pins.lcdOn := dataReg(11)
}
