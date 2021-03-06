/*
 * Start for OCP arbiter tests.
 *
 * Author: Martin Schoeberl (martin@jopdesign.com)
 *
 */

package ocp.test

import Chisel._
import ocp._
import io.SSRam32Ctrl


/*class Master(nr: Int, burstLength: Int) extends Module {

  val io = new Bundle {
    val port = new OcpBurstMasterPort(32, 32, burstLength)
  }

  val cntReg = Reg(init = UInt(0, width=8))

  io.port.M.Cmd := OcpCmd.IDLE
  io.port.M.DataValid := Bits(0)
  io.port.M.DataByteEn := Bits(15)

  cntReg := cntReg + UInt(1)
  switch(cntReg) {
    is(UInt(1)) {
      io.port.M.Cmd := OcpCmd.WR
      io.port.M.DataValid := Bits(1)
      when (io.port.S.CmdAccept === Bits(0)) {
        cntReg := cntReg
      }
    }
    is(UInt(2)) {
      io.port.M.DataValid := Bits(1)
    }
    is(UInt(3)) {
      io.port.M.DataValid := Bits(1)
    }
    // now we should be on our last word - wait for DVA
    is(UInt(4)) {
      io.port.M.DataValid := Bits(1)
      when (io.port.S.Resp != OcpResp.DVA) {
        cntReg := cntReg
      }
    }
    is(UInt(5)) { io.port.M.Cmd := OcpCmd.IDLE }
    is(UInt(6)) { io.port.M.Cmd := OcpCmd.RD }
  }

  io.port.M.Addr := (UInt(nr * 256) + cntReg).toBits()
  io.port.M.Data := (UInt(nr * 256 * 16) + cntReg).toBits()
} */

/** A top level to test the arbiter */
class NodeTdmArbiterTop() extends Module {

  val io = IO(new Bundle {
    val port = Vec.fill(3){new OcpBurstMasterPort(32, 32, 4)}
  })
  val CNT = 3
  //val arb = Module(new ocp.NodeTdmArbiter(CNT, 32, 32, 4))
  val mem = Module(new SSRam32Ctrl(21))
  val memMux = Module(new MemMuxIntf(3, 32, 32, 4))

  for (i <- 0 until CNT) {
    val m = Module(new Master(i, 4))
    val nodeID = UInt(i, width=6)
    val arb = Module(new ocp.NodeTdmArbiter(CNT, 32, 32, 4, 16))
    arb.io.master <> m.io.port
    arb.io.node := nodeID

    memMux.io.master(i) <> arb.io.slave
    io.port(i).M <> memMux.io.slave.M
  }

  mem.io.ocp <> memMux.io.slave

}

/*commented out Chisel3 tester has changed see https://github.com/schoeberl/chisel-examples/blob/master/TowardsChisel3.md 
class NodeTdmArbiterTester(dut: ocp.test.NodeTdmArbiterTop) extends Tester(dut) {
  val testVec = Array( OcpCmd.IDLE, OcpCmd.WR, OcpCmd.IDLE )

  for (i <- 0 until 35) {
    step(1)
  }
}

object NodeTdmArbiterTester {
  def main(args: Array[String]): Unit = {
    chiselMainTest(args, () => Module(new ocp.test.NodeTdmArbiterTop)) {
      f => new NodeTdmArbiterTester(f)
    }

  }
}*/
